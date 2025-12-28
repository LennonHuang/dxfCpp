#include "Entities/Axis.h"
#include <vector>
#include <cmath>

Axis::Axis(Type type, float length, float arrowSize)
    : _type(type), _length(length), _arrowSize(arrowSize) {
    // Color: X=red, Y=green
    if (_type == X) setColor(1.0f, 0.0f, 0.0f);
    else setColor(0.0f, 1.0f, 0.0f);
}

void Axis::createBuffers(QOpenGLFunctions_3_3_Core* f) {
    if (_type == X) {
        // Main line
        vertices = { 0.0f, 0.0f, _length, 0.0f };
        // Arrow
        vertices.insert(vertices.end(), {
            _length, 0.0f,
            _length - _arrowSize,  _arrowSize * 0.5f,
            _length, 0.0f,
            _length - _arrowSize, -_arrowSize * 0.5f
            });
    }
    else {
        vertices = { 0.0f, 0.0f, 0.0f, _length };
        vertices.insert(vertices.end(), {
            0.0f, _length,
            _arrowSize * 0.5f, _length - _arrowSize,
            0.0f, _length,
            -_arrowSize * 0.5f, _length - _arrowSize
            });
    }

    if (_vAO) deleteBuffers(f);

    f->glGenVertexArrays(1, &_vAO);
    f->glGenBuffers(1, &_vBO);
    f->glBindVertexArray(_vAO);
    f->glBindBuffer(GL_ARRAY_BUFFER, _vBO);
    f->glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    f->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    f->glEnableVertexAttribArray(0);
    f->glBindVertexArray(0);
}

void Axis::draw(QOpenGLFunctions_3_3_Core* f) const {
    if (!_vAO || !_vBO) return;

    f->glBindVertexArray(_vAO);

    // 2 for axis, 4 for arrow (2 lines)
    f->glDrawArrays(GL_LINES, 0, vertices.size()/2);
    f->glBindVertexArray(0);
}
