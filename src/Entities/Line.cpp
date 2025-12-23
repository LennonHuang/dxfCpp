#include "Entities/Line.h"
#include <iostream>

Line::Line(float x1, float y1, float x2, float y2)
    : vertices{ x1, y1, x2, y2 }
{
    // Don¡¯t create buffers here ¡ª no GL context yet
}

void Line::createBuffers(QOpenGLFunctions_3_3_Core* f)
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
    // Remember: VAO stores attribute config once called
    f->glEnableVertexAttribArray(0);
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
        2 * sizeof(float),
        reinterpret_cast<void*>(0));

    // Unbind for safety
    f->glBindBuffer(GL_ARRAY_BUFFER, 0);
    f->glBindVertexArray(0);
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
