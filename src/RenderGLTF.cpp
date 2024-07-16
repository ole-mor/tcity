#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "RenderGLTF.h"
#include <iostream>
#include <glm/glm.hpp>

void CreateMeshFromGLTF(const tinygltf::Model &model, const tinygltf::Mesh &gltfMesh, std::vector<Mesh> &meshes) {
    for (const auto &primitive : gltfMesh.primitives) {
        Mesh mesh;
        const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
        const tinygltf::Accessor &normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
        const tinygltf::Accessor &texAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];

        const tinygltf::BufferView &posView = model.bufferViews[posAccessor.bufferView];
        const tinygltf::BufferView &normView = model.bufferViews[normAccessor.bufferView];
        const tinygltf::BufferView &texView = model.bufferViews[texAccessor.bufferView];

        const tinygltf::Buffer &posBuffer = model.buffers[posView.buffer];
        const tinygltf::Buffer &normBuffer = model.buffers[normView.buffer];
        const tinygltf::Buffer &texBuffer = model.buffers[texView.buffer];

        for (size_t i = 0; i < posAccessor.count; ++i) {
            Vertex vertex;
            const float* pos = reinterpret_cast<const float*>(&posBuffer.data[posView.byteOffset + posAccessor.byteOffset + i * sizeof(glm::vec3)]);
            const float* norm = reinterpret_cast<const float*>(&normBuffer.data[normView.byteOffset + normAccessor.byteOffset + i * sizeof(glm::vec3)]);
            const float* tex = reinterpret_cast<const float*>(&texBuffer.data[texView.byteOffset + texAccessor.byteOffset + i * sizeof(glm::vec2)]);

            vertex.Position = glm::vec3(pos[0], pos[1], pos[2]);
            vertex.Normal = glm::vec3(norm[0], norm[1], norm[2]);
            vertex.TexCoords = glm::vec2(tex[0], tex[1]);

            mesh.vertices.push_back(vertex);
        }

        const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
        const tinygltf::BufferView &indexView = model.bufferViews[indexAccessor.bufferView];
        const tinygltf::Buffer &indexBuffer = model.buffers[indexView.buffer];

        for (size_t i = 0; i < indexAccessor.count; ++i) {
            const unsigned short* index = reinterpret_cast<const unsigned short*>(&indexBuffer.data[indexView.byteOffset + indexAccessor.byteOffset + i * sizeof(unsigned short)]);
            mesh.indices.push_back(*index);
        }

        glGenVertexArrays(1, &mesh.VAO);
        glGenBuffers(1, &mesh.VBO);
        glGenBuffers(1, &mesh.EBO);

        glBindVertexArray(mesh.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
        glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), &mesh.vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), &mesh.indices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);

        std::cout << "Mesh created with VAO: " << mesh.VAO << " and " << mesh.indices.size() << " indices.\n";

        meshes.push_back(mesh);
    }
}

void RenderMesh(const Mesh &mesh) {
    glBindVertexArray(mesh.VAO);
    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
