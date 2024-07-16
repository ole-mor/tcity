#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "Shader.h"
#include "RenderObject.h"
#include "LoadModel.h"
#include "RenderGLTF.h"

glm::mat4 projection;
Shader* shaderPtr = nullptr;
float offset = 0.0f;
float direction = 1.0f;

std::vector<Mesh> meshes;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    projection = glm::ortho(0.0f, static_cast<float>(width), 0.0f, static_cast<float>(height));
    if (shaderPtr) {
        shaderPtr->use();
        shaderPtr->setMat4("projection", projection);
    }
}

void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
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

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    Shader shader("../src/shaders/vertex_shader.glsl", "../src/shaders/fragment_shader.glsl");
    shaderPtr = &shader;

    tinygltf::Model model;
    if (!LoadGLTFModel(model, "../src/objects/untitled.glb")) {
        return -1;
    }

    for (const auto &gltfMesh : model.meshes) {
        CreateMeshFromGLTF(model, gltfMesh, meshes);
    }

    glm::mat4 view = glm::mat4(1.0f);
    int width, height; // Declare width and height
    glfwGetFramebufferSize(window, &width, &height);
    framebuffer_size_callback(window, width, height);

    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f));

    shader.use();
    shader.setMat4("view", view);

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.reloadIfModified();
        shader.use();

        for (const auto &mesh : meshes) {
            glm::mat4 model = glm::mat4(1.0f);
            shader.setMat4("model", model);
            RenderMesh(mesh);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
