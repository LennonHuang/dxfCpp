#pragma once

#include "Entities/Entity.h"
#include <vector>
#include <QOpenGLFunctions_3_3_Core>

class Line : public Entity
{
public:
    // Construct with two endpoints
    Line(float x1, float y1, float x2, float y2);

    // Draw with the given OpenGL functions resolver
    void draw(QOpenGLFunctions_3_3_Core* f) const override;
};
