#pragma once

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <tiny_gltf.h>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO, VBO, EBO;
};

void CreateMeshFromGLTF(const tinygltf::Model &model, const tinygltf::Mesh &gltfMesh, std::vector<Mesh> &meshes);
void RenderMesh(const Mesh &mesh);
