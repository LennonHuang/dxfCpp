#include "Entities/Entity.h"

// Base class constructor
Entity::Entity() = default;

// Create a VAO + VBO pair
// Derived classes should fill VBO after this call
void Entity::createBuffers(QOpenGLFunctions_3_3_Core* f)
{
    if (!f) return;

    f->glGenVertexArrays(1, &_vAO);
    f->glGenBuffers(1, &_vBO);
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
