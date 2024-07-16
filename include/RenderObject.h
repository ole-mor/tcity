#ifndef RENDEROBJECT_H
#define RENDEROBJECT_H

#include <glad/glad.h>
#include <cstddef>

class RenderObject {
public:
    RenderObject(float vertices[], std::size_t vertexCount);
    ~RenderObject();
    void draw();

private:
    unsigned int VAO, VBO;
};

#endif
