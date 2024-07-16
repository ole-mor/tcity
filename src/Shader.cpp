#include "Shader.h"
#include <sys/stat.h>
#include <chrono>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
    : vertexPath(vertexPath), fragmentPath(fragmentPath) {
    std::ifstream vShaderFile(vertexPath);
    std::ifstream fShaderFile(fragmentPath);

    if (!vShaderFile.is_open() || !fShaderFile.is_open()) {
        std::cerr << "Failed to open shader files: " << vertexPath << " and/or " << fragmentPath << std::endl;
        return;
    }

    std::stringstream vShaderStream, fShaderStream;
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();

    std::string vertexCode = vShaderStream.str();
    std::string fragmentCode = fShaderStream.str();

    compileAndLinkShaders(vertexCode, fragmentCode);

    vertexShaderLastWriteTime = getLastWriteTime(vertexPath);
    fragmentShaderLastWriteTime = getLastWriteTime(fragmentPath);
}

void Shader::reloadIfModified() {
    checkForModification();
}

std::time_t Shader::getLastWriteTime(const std::string& path) const {
    struct stat fileInfo;
    if (stat(path.c_str(), &fileInfo) == 0) {
        return fileInfo.st_mtime;
    }
    return 0;
}

void Shader::checkForModification() {
    std::time_t currentVertexShaderWriteTime = getLastWriteTime(vertexPath);
    std::time_t currentFragmentShaderWriteTime = getLastWriteTime(fragmentPath);

    if (currentVertexShaderWriteTime != vertexShaderLastWriteTime || currentFragmentShaderWriteTime != fragmentShaderLastWriteTime) {
        std::ifstream vShaderFile(vertexPath);
        std::ifstream fShaderFile(fragmentPath);

        if (!vShaderFile.is_open() || !fShaderFile.is_open()) {
            std::cerr << "Failed to open shader files: " << vertexPath << " and/or " << fragmentPath << std::endl;
            return;
        }

        std::stringstream vShaderStream, fShaderStream;
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        std::string vertexCode = vShaderStream.str();
        std::string fragmentCode = fShaderStream.str();

        compileAndLinkShaders(vertexCode, fragmentCode);

        vertexShaderLastWriteTime = currentVertexShaderWriteTime;
        fragmentShaderLastWriteTime = currentFragmentShaderWriteTime;
    }
}

void Shader::compileAndLinkShaders(const std::string& vertexCode, const std::string& fragmentCode) {
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    int success;
    char infoLog[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    } else {
        std::cout << "Vertex shader compiled successfully.\n";
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    } else {
        std::cout << "Fragment shader compiled successfully.\n";
    }

    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    } else {
        std::cout << "Shader program linked successfully.\n";
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}
