#pragma once

#include "Entities/Entity.h"
#include <vector>
#include <glm/vec2.hpp>
#include "Dxfloader.h"

struct PolylineVertex {
    glm::vec2 position;
    float bulge;

    PolylineVertex(float x, float y, float bulge)
        : position(x, y), bulge(bulge) {}
};

class Polyline : public Entity {
public:
    Polyline() = default;
    explicit Polyline(const DRW_LWPolyline& plydata);

    // Implement pure virtual function from Entity
    void draw(QOpenGLFunctions_3_3_Core* f) const override;

    std::string getType() const override { return "Polyline"; }

private:
    std::vector<PolylineVertex> m_plyvertices;
};
