#include <iostream>
#include "tiny_gltf.h"

bool LoadGLTFModel(tinygltf::Model &model, const std::string &filename) {
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    std::cout << "Loading GLTF file: " << filename << std::endl;
    
    bool res = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
    if (!warn.empty()) {
        std::cout << "Warn: " << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << "Err: " << err << std::endl;
    }

    if (!res) {
        std::cerr << "Failed to load glTF: " << filename << std::endl;
    } else {
        std::cout << "GLTF model loaded successfully" << std::endl;
    }

    return res;
}
