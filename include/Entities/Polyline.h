#pragma once

#include "Entities/Entity.h"
#include "MathHelper.h"
#include <vector>
#include <glm/vec2.hpp>
#include "Dxfloader.h"

class Polyline : public Entity {
public:
    Polyline() = default;
    explicit Polyline(const DRW_LWPolyline& plydata);

    // Implement pure virtual function from Entity
    void draw(QOpenGLFunctions_3_3_Core* f) const override;

    std::string getType() const override { return "Polyline"; }

    // Access original vertices with bulge info (for accurate arc intersection)
    const std::vector<PolylineVertex>& getPlyVertices() const { return m_plyvertices; }

private:
    std::vector<PolylineVertex> m_plyvertices;
};
