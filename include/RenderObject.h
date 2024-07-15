#ifndef RENDEROBJECT_H
#define RENDEROBJECT_H

#include <glad/glad.h>
#include <cstddef>  // Include this for size_t

class RenderObject {
public:
    unsigned int VAO, VBO;

    RenderObject(float vertices[], std::size_t vertexCount); // Use std::size_t
    ~RenderObject();
    void draw();
};

#endif
