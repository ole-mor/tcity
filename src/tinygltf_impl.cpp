#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "tiny_gltf.h"
#include "stb_image.h"
#include "stb_image_write.h"

// Implement the required callback functions

bool FileExists(const std::string &abs_filename, void *) {
    FILE *file = fopen(abs_filename.c_str(), "rb");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

bool ReadWholeFile(std::vector<unsigned char> *out, std::string *err, const std::string &filepath, void *) {
    FILE *file = fopen(filepath.c_str(), "rb");
    if (!file) {
        if (err) {
            *err = "File not found : " + filepath;
        }
        return false;
    }

    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    out->resize(fileSize);
    size_t readSize = fread(out->data(), 1, fileSize, file);
    fclose(file);

    if (readSize != fileSize) {
        if (err) {
            *err = "Read file error : " + filepath;
        }
        return false;
    }

    return true;
}

bool WriteWholeFile(std::string *err, const std::string &filepath, const std::vector<unsigned char> &contents, void *) {
    FILE *file = fopen(filepath.c_str(), "wb");
    if (!file) {
        if (err) {
            *err = "File open error : " + filepath;
        }
        return false;
    }

    size_t writtenSize = fwrite(contents.data(), 1, contents.size(), file);
    fclose(file);

    if (writtenSize != contents.size()) {
        if (err) {
            *err = "Write file error : " + filepath;
        }
        return false;
    }

    return true;
}
