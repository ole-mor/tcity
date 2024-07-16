#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader {
public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath);

    void use() const {
        glUseProgram(ID);
    }

    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, glm::value_ptr(value));
    }

    void reloadIfModified();
private:
    std::string vertexPath;
    std::string fragmentPath;
    std::time_t vertexShaderLastWriteTime;
    std::time_t fragmentShaderLastWriteTime;

    std::time_t getLastWriteTime(const std::string& path) const;
    void checkForModification();
    void compileAndLinkShaders(const std::string& vertexCode, const std::string& fragmentCode);
};

#endif // SHADER_H
