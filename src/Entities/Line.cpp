#include "Entities/Line.h"
#include <iostream>

Line::Line(float x1, float y1, float x2, float y2)
{
    // Don¡¯t create buffers here ¡ª no GL context yet
    vertices.insert(vertices.end(), { x1, y1, x2, y2 });
}

void Line::draw(QOpenGLFunctions_3_3_Core* f) const
{
    if (_vAO == 0) {
        std::cerr << "Line::draw(): VAO not created\n";
        return;
    }

    // Bind VAO and draw lines
    f->glBindVertexArray(_vAO);
    f->glDrawArrays(GL_LINES, 0, 2);
    f->glBindVertexArray(0);
}
