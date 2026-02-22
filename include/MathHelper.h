#pragma once

#include <glm/glm.hpp>
#include <vector>

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
    // Returns the center as glm::vec2.
    static glm::vec2 CenterFromBulge(const glm::vec2& p1, const glm::vec2& p2, float bulge) {
        if (IsZero(bulge)) {
            // No arc, just return midpoint
            return (p1 + p2) * 0.5f;
        }

        glm::vec2 chord = p2 - p1;
        float chordLen = glm::length(chord);

        // Distance from midpoint to center
		// derived from bulge definition
        float h = chordLen / (4.0f * bulge);

        // Midpoint of the chord
        glm::vec2 mid = (p1 + p2) * 0.5f;

        // Perpendicular direction
        glm::vec2 perp(-chord.y, chord.x);
        perp = glm::normalize(perp);

        // Return center of the arc
        return mid + perp * h;
    }

    // Test intersection of two line segments.
    // Returns true if they intersect, with parametric positions tA, tB and the intersection point.
    static bool SegmentIntersect(
        const glm::vec2& a1, const glm::vec2& a2,
        const glm::vec2& b1, const glm::vec2& b2,
        float& tA, float& tB, glm::vec2& point);

    // Find all intersection points between two polylines.
    // Returns results sorted by vertex order along polyline A.
    // vertsA/vertsB are flat arrays: [x0, y0, x1, y1, ...]
    static std::vector<IntersectResult> PolylineIntersect(
        const std::vector<float>& vertsA, bool closedA,
        const std::vector<float>& vertsB, bool closedB);
};
