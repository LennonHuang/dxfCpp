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

bool Entity::hitTest(float worldX, float worldY, float tolerance) const
{
    // Walk vertex pairs (x1,y1, x2,y2, ...) as line segments
    int vertCount = vertices.size() / 2;
    for (int i = 0; i < vertCount - 1; ++i) {
        float x1 = vertices[i * 2], y1 = vertices[i * 2 + 1];
        float x2 = vertices[(i + 1) * 2], y2 = vertices[(i + 1) * 2 + 1];

        // Point-to-segment distance
        float dx = x2 - x1, dy = y2 - y1;
        float lenSq = dx * dx + dy * dy;
        float t = (lenSq > 0) ? std::clamp(((worldX - x1) * dx + (worldY - y1) * dy) / lenSq, 0.f, 1.f) : 0.f;
        float px = x1 + t * dx, py = y1 + t * dy;
        float dist = std::sqrt((worldX - px) * (worldX - px) + (worldY - py) * (worldY - py));

        if (dist <= tolerance) return true;
    }
    return false;
}
