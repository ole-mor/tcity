#ifndef COMMONTYPES_H
#define COMMONTYPES_H

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>

// Define the Vertex structure
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};

// Define the Mesh structure
struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint VAO, VBO, EBO;
    int nodeIndex; // Add this member to store the node index
};

#endif // COMMONTYPES_H
