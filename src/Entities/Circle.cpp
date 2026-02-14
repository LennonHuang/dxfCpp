#include "Entities/Circle.h"
#include <cmath>
#include <iostream>

Circle::Circle(float cx, float cy, float radius, int segments)
{
    _closed = true;
    vertices.reserve(segments * 2);

    const float step = 2.0f * static_cast<float>(M_PI) / segments;

    for (int i = 0; i < segments; ++i)
    {
        float angle = i * step;
        float x = cx + radius * std::cos(angle);
        float y = cy + radius * std::sin(angle);

        vertices.push_back(x);
        vertices.push_back(y);
    }
}

void Circle::draw(QOpenGLFunctions_3_3_Core* f) const
{
    if (_vAO == 0)
    {
        std::cerr << "Circle::draw(): VAO not created\n";
        return;
    }

    f->glBindVertexArray(_vAO);
    f->glDrawArrays(GL_LINE_LOOP, 0, static_cast<GLsizei>(vertices.size() / 2));
    f->glBindVertexArray(0);
}
