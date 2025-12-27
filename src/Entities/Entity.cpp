#include "Entities/Entity.h"

// Base class constructor
Entity::Entity() = default;

void Entity::createBuffers(QOpenGLFunctions_3_3_Core* f)
{
    if (!f) return;

    // Generate VAO/VBO
    f->glGenVertexArrays(1, &_vAO);
    f->glGenBuffers(1, &_vBO);

    // Bind VAO, then VBO, upload, and set attribute
    f->glBindVertexArray(_vAO);

    f->glBindBuffer(GL_ARRAY_BUFFER, _vBO);
    f->glBufferData(GL_ARRAY_BUFFER,
        vertices.size() * sizeof(float),
        vertices.data(),
        GL_STATIC_DRAW);

    // Attribute 0: 2 floats per vertex (x,y)  
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
        2 * sizeof(float),
        reinterpret_cast<void*>(0));
    f->glEnableVertexAttribArray(0);

    // Unbind for safety
    f->glBindBuffer(GL_ARRAY_BUFFER, 0);
    f->glBindVertexArray(0);
}

// Delete VAO + VBO
void Entity::deleteBuffers(QOpenGLFunctions_3_3_Core* f)
{
    if (!f) return;

    if (_vBO != 0) {
        f->glDeleteBuffers(1, &_vBO);
        _vBO = 0;
    }
    if (_vAO != 0) {
        f->glDeleteVertexArrays(1, &_vAO);
        _vAO = 0;
    }
}
