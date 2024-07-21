#ifndef SIMPLECUBE_H
#define SIMPLECUBE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "Shader.h"

class SimpleCube {
public:
    SimpleCube(const glm::vec3 &position, const glm::vec4 &color, float metallic, float roughness);
    ~SimpleCube();
    void Render(Shader &shader);

private:
    GLuint VAO, VBO;
    glm::vec3 position;
    glm::vec4 color; // Material color
    float metallic;  // Material metallic factor
    float roughness; // Material roughness factor
    void setupCube();
};

#endif // SIMPLECUBE_H