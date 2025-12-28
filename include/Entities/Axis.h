#pragma once
#include "Entities/Entity.h"

class Axis : public Entity {
public:
    enum Type { X, Y };
    Axis(Type type, float length = 1000.0f, float arrowSize = 10.0f);

    void createBuffers(QOpenGLFunctions_3_3_Core* f) override;
    void draw(QOpenGLFunctions_3_3_Core* f) const override;

private:
    Type _type;
    float _length;
    float _arrowSize;
};
