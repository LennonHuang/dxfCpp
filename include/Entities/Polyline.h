#pragma once

#include "Entities/Entity.h"
#include <vector>
#include <glm/vec2.hpp>
#include "DxfLoader.h"

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
    Polyline(const std::vector<PolylineVertex>& verts, bool closed);

    // Implement pure virtual function from Entity
    void draw(QOpenGLFunctions_3_3_Core* f) const override;

    std::string getType() const override { return "Polyline"; }

    const std::vector<PolylineVertex>& getPolyVertices() const { return m_plyvertices; }
    bool getIsClosed() const { return isClosed; }

private:
    std::vector<PolylineVertex> m_plyvertices;
    bool isClosed = false;
};
