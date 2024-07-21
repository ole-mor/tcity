#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <vector>
#include <tiny_gltf.h>
#include "Shader.h"
#include "RenderObject.h"
#include "LoadModel.h"
#include "RenderGLTF.h"
#include "SimpleCube.h"

glm::mat4 projection;
glm::vec3 cameraPos = glm::vec3(3.0f, 3.0f, -12.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight;
float yaw = 110.0f;
float pitch = -20.0f;
float cameraSpeed = 0.05f;
float rotationSpeed = 1.0f;

float currentTime = 0.0f;
float lastFrameTime = 0.0f;
const float animationDuration = 78.0f; // Total duration of the animation in frames
const float frameDuration = 1.0f / 24.0f; // Duration of each frame assuming 24 FPS

Shader* shaderPtr = nullptr;

struct AnimationData {
    std::vector<float> times;
    std::vector<glm::vec3> translations;
    std::vector<glm::vec3> scales;
};

struct Material {
    glm::vec4 baseColor;
    float metallic;
    float roughness;
};

struct Light {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

class SpawnObject {
public:
    std::vector<Mesh> meshes;
    std::vector<AnimationData> animations;
    std::vector<Material> materials; // Store materials
    std::vector<Light> lights;       // Store lights
    glm::vec3 position;
    glm::mat4 modelMatrix;
    float animationTime;
    float animationSpeed;

    SpawnObject(const std::string &path, const glm::vec3 &initialPosition)
        : position(initialPosition), animationTime(0.0f), animationSpeed(1.0f) {
        tinygltf::Model model;
        if (!LoadGLTFModel(model, path)) {
            std::cerr << "Failed to load model." << std::endl;
            return;
        }
        LoadAnimationData(model);
        LoadMaterialData(model); // Load materials
        LoadLightData(model);    // Load lights
        for (const auto &gltfMesh : model.meshes) {
            CreateMeshFromGLTF(model, gltfMesh, meshes);
        }
        modelMatrix = glm::mat4(1.0f);
    }

    void LoadAnimationData(const tinygltf::Model &model) {
        for (const auto &animation : model.animations) {
            AnimationData animData;
            for (const auto &sampler : animation.samplers) {
                const auto &inputAccessor = model.accessors[sampler.input];
                const auto &outputAccessor = model.accessors[sampler.output];
                const auto &inputBufferView = model.bufferViews[inputAccessor.bufferView];
                const auto &inputBuffer = model.buffers[inputBufferView.buffer];
                const auto &outputBufferView = model.bufferViews[outputAccessor.bufferView];
                const auto &outputBuffer = model.buffers[outputBufferView.buffer];

                std::vector<float> inputData(inputAccessor.count);
                std::vector<glm::vec3> outputVec3Data(outputAccessor.count);

                std::memcpy(inputData.data(), inputBuffer.data.data() + inputBufferView.byteOffset + inputAccessor.byteOffset, inputAccessor.count * sizeof(float));
                std::memcpy(outputVec3Data.data(), outputBuffer.data.data() + outputBufferView.byteOffset + outputAccessor.byteOffset, outputAccessor.count * sizeof(glm::vec3));

                animData.times = inputData;

                for (const auto &channel : animation.channels) {
                    if (channel.sampler == &sampler - &animation.samplers[0]) {
                        if (channel.target_path == "translation") {
                            animData.translations = outputVec3Data;
                        } else if (channel.target_path == "scale") {
                            animData.scales = outputVec3Data;
                        }
                    }
                }
            }
            animations.push_back(animData);
        }
    }

    void LoadMaterialData(const tinygltf::Model &model) {
        for (const auto &gltfMaterial : model.materials) {
            Material material;
            auto baseColorFactor = gltfMaterial.values.find("baseColorFactor");
            if (baseColorFactor != gltfMaterial.values.end()) {
                const auto &color = baseColorFactor->second.number_array;
                material.baseColor = glm::vec4(color[0], color[1], color[2], color[3]);
            } else {
                material.baseColor = glm::vec4(1.0f);
            }

            auto metallicFactor = gltfMaterial.values.find("metallicFactor");
            if (metallicFactor != gltfMaterial.values.end()) {
                material.metallic = static_cast<float>(metallicFactor->second.Factor());
            } else {
                material.metallic = 1.0f;
            }

            auto roughnessFactor = gltfMaterial.values.find("roughnessFactor");
            if (roughnessFactor != gltfMaterial.values.end()) {
                material.roughness = static_cast<float>(roughnessFactor->second.Factor());
            } else {
                material.roughness = 1.0f;
            }

            materials.push_back(material);
        }
    }

    void LoadLightData(const tinygltf::Model &model) {
        for (const auto &node : model.nodes) {
            if (node.extensions.find("KHR_lights_punctual") != node.extensions.end()) {
                const auto &lightIndex = node.extensions.at("KHR_lights_punctual").Get("light").Get<int>();
                const tinygltf::Light &light = model.lights[lightIndex];

                Light lightData;
                lightData.position = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
                lightData.color = glm::vec3(light.color[0], light.color[1], light.color[2]);
                lightData.intensity = light.intensity;

                lights.push_back(lightData);
            }
        }
    }

    void Update(float deltaTime) {
        animationTime += deltaTime * animationSpeed;
        if (!animations.empty()) {
            const auto &animData = animations[0];
            if (!animData.times.empty()) {
                animationTime = fmod(animationTime, animData.times.back());
                UpdateModelTransformation(modelMatrix, animData.times, animData.translations, animData.scales, animationTime);
            }
        }
    }

    void Render(Shader &shader) {
        glm::mat4 modelWithInitialPosition = glm::translate(modelMatrix, position);
        shader.setMat4("model", modelWithInitialPosition);
        
        // Set materials
        if (!materials.empty()) {
            shader.setVec4("material.baseColor", materials[0].baseColor);
            shader.setFloat("material.metallic", materials[0].metallic);
            shader.setFloat("material.roughness", materials[0].roughness);
        }

        // Set lights
        for (size_t i = 0; i < lights.size(); ++i) {
            shader.setVec3("lights[" + std::to_string(i) + "].position", lights[i].position);
            shader.setVec3("lights[" + std::to_string(i) + "].color", lights[i].color);
            shader.setFloat("lights[" + std::to_string(i) + "].intensity", lights[i].intensity);
        }
        shader.setInt("numLights", static_cast<int>(lights.size()));

        for (auto &mesh : meshes) {
            RenderMesh(mesh);
        }
    }

private:
    glm::vec3 Lerp(const glm::vec3 &a, const glm::vec3 &b, float t) {
        return a + t * (b - a);
    }

    void UpdateModelTransformation(glm::mat4 &modelMatrix,
                                   const std::vector<float> &times,
                                   const std::vector<glm::vec3> &translations,
                                   const std::vector<glm::vec3> &scales,
                                   float animationTime) {
        size_t prevKeyframeIndex = 0;
        size_t nextKeyframeIndex = 1;

        for (size_t i = 0; i < times.size() - 1; ++i) {
            if (animationTime >= times[i] && animationTime < times[i + 1]) {
                prevKeyframeIndex = i;
                nextKeyframeIndex = i + 1;
                break;
            }
        }

        float t = (animationTime - times[prevKeyframeIndex]) / (times[nextKeyframeIndex] - times[prevKeyframeIndex]);

        glm::vec3 interpolatedTranslation = Lerp(translations[prevKeyframeIndex], translations[nextKeyframeIndex], t);
        glm::vec3 interpolatedScale = Lerp(scales[prevKeyframeIndex], scales[nextKeyframeIndex], t);

        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, interpolatedTranslation);
        modelMatrix = glm::scale(modelMatrix, interpolatedScale);
    }
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    if (shaderPtr) {
        shaderPtr->use();
        shaderPtr->setMat4("projection", projection);
    }
}

void updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);

    cameraRight = glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= cameraRight * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += cameraRight * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        pitch += rotationSpeed;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        pitch -= rotationSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        yaw -= rotationSpeed;
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        yaw += rotationSpeed;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    updateCameraVectors();
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "tcity", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    Shader shader("../src/shaders/vertex_shader.glsl", "../src/shaders/fragment_shader.glsl");
    shaderPtr = &shader;

    std::vector<SpawnObject> objects;
    objects.emplace_back("../src/objects/untitled-cubered-material.glb", glm::vec3(-2.0f, 0.0f, -5.0f));
    objects.emplace_back("../src/objects/untitled-cube-anim.glb", glm::vec3(2.0f, 0.0f, -5.0f));

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);

    lastFrameTime = glfwGetTime();

    SimpleCube simpleCube(glm::vec3(0.0f, 0.0f, -5.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), 0.5f, 0.5f);

    while (!glfwWindowShouldClose(window)) {
        currentTime = glfwGetTime();
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        processInput(window);

        glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.reloadIfModified();
        shader.use();

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

            // Set light properties
        glm::vec3 lightColor(1.0f, 1.0f, 1.0f); // white light
        shader.setVec3("lights[0].color", lightColor);

        glm::vec3 lightPos(0.0f, -1.0f, -10.0f); // light coming from above
        shader.setVec3("lights[0].position", lightPos);

        float lightIntensity = 1.0f; // intensity of the light
        shader.setFloat("lights[0].intensity", lightIntensity);

        for (auto &obj : objects) {
            obj.Update(deltaTime);
            obj.Render(shader);
        }

        // simpleCube.Render(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return -1;
}
