#include "Entities/Entity.h"
#include <cmath>
#include <algorithm>

// Base class constructor
Entity::Entity() = default;

// Point-to-line-segment distance
static float pointToSegmentDist(float px, float py, float ax, float ay, float bx, float by)
{
    float dx = bx - ax;
    float dy = by - ay;
    float lenSq = dx * dx + dy * dy;
    if (lenSq < 1e-12f) {
        // Degenerate segment (point)
        float ex = px - ax;
        float ey = py - ay;
        return std::sqrt(ex * ex + ey * ey);
    }
    float t = ((px - ax) * dx + (py - ay) * dy) / lenSq;
    t = std::clamp(t, 0.0f, 1.0f);
    float cx = ax + t * dx;
    float cy = ay + t * dy;
    float ex = px - cx;
    float ey = py - cy;
    return std::sqrt(ex * ex + ey * ey);
}

bool Entity::hitTest(float wx, float wy, float tolerance) const
{
    size_t count = vertices.size() / 2;
    if (count < 2) return false;

    // Check each consecutive segment
    for (size_t i = 0; i + 1 < count; ++i) {
        float dist = pointToSegmentDist(wx, wy,
            vertices[i * 2], vertices[i * 2 + 1],
            vertices[(i + 1) * 2], vertices[(i + 1) * 2 + 1]);
        if (dist <= tolerance) return true;
    }

    // For closed shapes, check the closing segment (last → first)
    if (_closed && count > 2) {
        float dist = pointToSegmentDist(wx, wy,
            vertices[(count - 1) * 2], vertices[(count - 1) * 2 + 1],
            vertices[0], vertices[1]);
        if (dist <= tolerance) return true;
    }

    return false;
}

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
