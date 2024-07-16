#define GLM_ENABLE_EXPERIMENTAL

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include "Shader.h"
#include "RenderObject.h"
#include "LoadModel.h"
#include "RenderGLTF.h"

glm::mat4 projection;
glm::vec3 cameraPos = glm::vec3(3.0f, -5.0f, -7.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight;
float yaw = 90.0f;
float pitch = 0.0f;
float cameraSpeed = 0.05f;
float rotationSpeed = 1.0f;

Shader* shaderPtr = nullptr;
std::vector<Mesh> meshes;

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

    // Update the cameraRight and cameraUp vectors
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

    tinygltf::Model model;
    if (!LoadGLTFModel(model, "../src/objects/untitled.glb")) {
        std::cerr << "Failed to load model." << std::endl;
        return -1;
    }

    std::cout << "GLTF model loaded successfully" << std::endl;

    for (const auto &gltfMesh : model.meshes) {
        CreateMeshFromGLTF(model, gltfMesh, meshes);
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.reloadIfModified();
        shader.use();

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        // Render the first cube
        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, glm::vec3(-2.0f, 0.0f, -5.0f)); // Translate the first cube to the left
        shader.setMat4("model", model1);
        RenderMesh(meshes[0]); // Assuming meshes[0] is the cube

        // Render the second cube
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(2.0f, 0.0f, -5.0f)); // Translate the second cube to the right
        shader.setMat4("model", model2);
        RenderMesh(meshes[0]); // Assuming meshes[0] is the cube

        glm::mat4 model3 = glm::mat4(1.0f);
        model3 = glm::translate(model3, glm::vec3(0.0f, 0.0f, -3.0f)); // Translate the second cube to the right
        shader.setMat4("model", model3);
        RenderMesh(meshes[0]); // Assuming meshes[0] is the cube

        glm::mat4 model4 = glm::mat4(1.0f);
        model4 = glm::translate(model4, glm::vec3(0.0f, 0.0f, -1.0f)); // Translate the second cube to the right
        shader.setMat4("model", model4);
        RenderMesh(meshes[0]); // Assuming meshes[0] is the cube


        glm::mat4 model5 = glm::mat4(1.0f);
        model5 = glm::translate(model5, glm::vec3(0.0f, 0.0f, 1.0f)); // Translate the second cube to the right
        shader.setMat4("model", model5);
        RenderMesh(meshes[0]); // Assuming meshes[0] is the cube

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
