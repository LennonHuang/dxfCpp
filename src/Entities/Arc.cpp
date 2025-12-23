#include "Entities/Arc.h"
#include <cmath>

Arc::Arc(float cx, float cy, float radius,
    float startAngle, float endAngle,
    int segments)
{
    // If end angle is less than start, wrap around
    if (endAngle < startAngle) {
        endAngle += 2.0f * static_cast<float>(M_PI);
    }

    float angleRange = endAngle - startAngle;
    float step = angleRange / static_cast<float>(segments);

    vertices.reserve((segments + 1) * 2);
    for (int i = 0; i <= segments; ++i) {
        float angle = startAngle + i * step;
        float x = cx + radius * std::cos(angle);
        float y = cy + radius * std::sin(angle);
        vertices.push_back(x);
        vertices.push_back(y);
    }
}

void Arc::draw(QOpenGLFunctions_3_3_Core* f) const
{
    if (_vAO == 0) return;

    f->glBindVertexArray(_vAO);
    f->glDrawArrays(GL_LINE_STRIP, 0, static_cast<GLsizei>(vertices.size() / 2));
    f->glBindVertexArray(0);
}
