#pragma once

#include <vector>
#include <QOpenGLFunctions_3_3_Core>
#include "Entities/Entity.h"

class Arc : public Entity
{
public:
    Arc(float cx, float cy, float radius, float startAngle, float endAngle, int segments = 64);
    void draw(QOpenGLFunctions_3_3_Core* f) const override;
};