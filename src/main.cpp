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

class SpawnObject {
public:
    std::vector<Mesh> meshes;
    std::vector<AnimationData> animations;
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
    objects.emplace_back("../src/objects/untitled-cube-anim.glb", glm::vec3(-2.0f, 0.0f, -5.0f));
    objects.emplace_back("../src/objects/untitled-cube-anim.glb", glm::vec3(2.0f, 0.0f, -5.0f));

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);

    lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window)) {
        currentTime = glfwGetTime();
        float deltaTime = currentTime - lastFrameTime;
        lastFrameTime = currentTime;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.reloadIfModified();
        shader.use();

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        for (auto &obj : objects) {
            obj.Update(deltaTime);
            obj.Render(shader);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return -1;
}
