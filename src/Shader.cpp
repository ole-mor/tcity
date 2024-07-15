#include "Shader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
    : vertexPath(vertexPath), fragmentPath(fragmentPath) {
    vertexLastModifiedTime = getLastModifiedTime(vertexPath);
    fragmentLastModifiedTime = getLastModifiedTime(fragmentPath);
    compileAndLinkShaders();
}

void Shader::use() {
    glUseProgram(ID);
}

void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const {
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
}

void Shader::reloadIfModified() {
    auto newVertexTime = getLastModifiedTime(vertexPath.c_str());
    auto newFragmentTime = getLastModifiedTime(fragmentPath.c_str());

    if (newVertexTime > vertexLastModifiedTime || newFragmentTime > fragmentLastModifiedTime) {
        vertexLastModifiedTime = newVertexTime;
        fragmentLastModifiedTime = newFragmentTime;
        compileAndLinkShaders();
    }
}

std::string Shader::readFile(const char* filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void Shader::checkCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}

std::chrono::time_point<std::chrono::system_clock> Shader::getLastModifiedTime(const char* filePath) {
    struct stat fileStat;
    if (stat(filePath, &fileStat) == 0) {
        return std::chrono::system_clock::from_time_t(fileStat.st_mtime);
    }
    return std::chrono::system_clock::now();
}

void Shader::compileAndLinkShaders() {
    std::string vertexCode = readFile(vertexPath.c_str());
    std::string fragmentCode = readFile(fragmentPath.c_str());

    // Debugging: Print the shader source code
    std::cout << "Vertex Shader Source Code:\n" << vertexCode << std::endl;
    std::cout << "Fragment Shader Source Code:\n" << fragmentCode << std::endl;

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    checkCompileErrors(vertex, "VERTEX");

    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    checkCompileErrors(fragment, "FRAGMENT");

    // Shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // Delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}
