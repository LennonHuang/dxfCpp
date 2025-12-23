#pragma once

#include <vector>
#include <QOpenGLFunctions_3_3_Core>
#include "Entities/Entity.h"

class Circle : public Entity
{
public:
    Circle(float cx, float cy, float radius, int segments = 64);

    void draw(QOpenGLFunctions_3_3_Core* f) const;
};
