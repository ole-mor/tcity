#pragma once

#include <tiny_gltf.h>
#include <string>

bool LoadGLTFModel(tinygltf::Model &model, const std::string &filename);
