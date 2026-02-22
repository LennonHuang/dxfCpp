#pragma once

#include <glm/glm.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <vector>
#include <cmath>

struct PolylineVertex {
    glm::vec2 position;
    float bulge;

    PolylineVertex(float x, float y, float bulge)
        : position(x, y), bulge(bulge) {}
};

struct PolylineSegment {
    glm::vec2 start;
    glm::vec2 end;
    bool isArc;
    // Arc-only fields (valid when isArc == true)
    glm::vec2 center;
    float radius;
    float startAngle;   // angle at 'start' relative to center
    float endAngle;     // angle at 'end' relative to center
    // sweep = endAngle - startAngle  (positive = CCW, negative = CW)
};

struct IntersectResult {
    glm::vec2 point;
    int segIndexA;   // which segment of polyline A
    float paramA;    // parametric position [0,1] on that segment
    int segIndexB;   // which segment of polyline B
    float paramB;    // parametric position [0,1] on that segment
};

class MathHelper
{
public:
	static constexpr float EPSILON = 1e-6f;
	static bool IsZero(float value) {
		return std::abs(value) <= EPSILON;
	}

    // Calculates the center of an arc defined by two points and a bulge value.
    static glm::vec2 CenterFromBulge(const glm::vec2& p1, const glm::vec2& p2, float bulge) {
        if (IsZero(bulge)) {
            return (p1 + p2) * 0.5f;
        }

        glm::vec2 chord = p2 - p1;
        float chordLen = glm::length(chord);

        float h = chordLen / (4.0f * bulge);

        glm::vec2 mid = (p1 + p2) * 0.5f;

        glm::vec2 perp(-chord.y, chord.x);
        perp = glm::normalize(perp);

        return mid + perp * h;
    }

    // Build typed segments (line or arc) from polyline vertices with bulge info.
    static std::vector<PolylineSegment> BuildSegments(
        const std::vector<PolylineVertex>& verts, bool closed);

    // Line-segment vs line-segment intersection.
    static bool SegmentIntersect(
        const glm::vec2& a1, const glm::vec2& a2,
        const glm::vec2& b1, const glm::vec2& b2,
        float& tA, float& tB, glm::vec2& point);

    // Polyline intersection using flat vertex arrays (line segments only).
    static std::vector<IntersectResult> PolylineIntersect(
        const std::vector<float>& vertsA, bool closedA,
        const std::vector<float>& vertsB, bool closedB);

    // Accurate polyline intersection with arc support.
    // Uses PolylineSegment lists built from BuildSegments().
    static std::vector<IntersectResult> PolylineIntersect(
        const std::vector<PolylineSegment>& segsA,
        const std::vector<PolylineSegment>& segsB);
};
