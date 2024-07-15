#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <chrono>

class Shader {
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath);
    void use();
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setVec4(const std::string &name, float x, float y, float z, float w) const;  // New method
    void reloadIfModified();

private:
    std::string vertexPath;
    std::string fragmentPath;
    std::chrono::time_point<std::chrono::system_clock> vertexLastModifiedTime;
    std::chrono::time_point<std::chrono::system_clock> fragmentLastModifiedTime;

    std::string readFile(const char* filePath);
    void checkCompileErrors(unsigned int shader, std::string type);
    std::chrono::time_point<std::chrono::system_clock> getLastModifiedTime(const char* filePath);
    void compileAndLinkShaders();
};

#endif
