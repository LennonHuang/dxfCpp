#include <glm/glm.hpp>
#include <Entities/Polyline.h>

class AutoDxfHelper
{
public:
	static constexpr float EPSILON = 1e-6f;

	static bool IsZero(float value) {
		return std::abs(value) <= EPSILON;
	}

    struct PolylineSegment {
        glm::vec2 start;
        glm::vec2 end;
        float bulge;          // 0 = line, else arc
        int originalIndex;    // index in source polyline (for ordering)

        bool isArc() const { return !IsZero(bulge); }
    };

    struct IntersectionPoint {
        glm::vec2 point;
        int segmentIndex;     // which segment of polyline A
        float parameter;      // ordering key along that segment
        //   line: parametric t [0,1]
        //   arc:  normalized angle [0,1]
    };

    // Calculates the center of an arc defined by two points and a bulge value.
    // Returns the center as glm::vec2.
	static glm::vec2 CenterFromBulge(const glm::vec2& p1, const glm::vec2& p2, float bulge);

    static float RadiusFromBulge(const glm::vec2& p1, const glm::vec2& p2, float bulge);

    // Given aQ check whether its in [a1, a2]
    static bool AngleOnArc(float a1, float a2, float aQ, float bulge);

    static std::pair<float, float> SplitBulge(
        const glm::vec2& p1, const glm::vec2& p2, float bulge,
        const glm::vec2& splitPoint);

    static std::vector<IntersectionPoint> IntersectSegments(
        const PolylineSegment& a, const PolylineSegment& b);

    static std::vector<IntersectionPoint> IntersectLineLine(
        const glm::vec2& p1, const glm::vec2& p2,
        const glm::vec2& q1, const glm::vec2& q2);

    static std::vector<IntersectionPoint> IntersectLineArc(
        const glm::vec2& lp1, const glm::vec2& lp2,      // line
        const glm::vec2& ap1, const glm::vec2& ap2, float bulge); // arc

    static std::vector<IntersectionPoint> IntersectArcArc(
        const glm::vec2& a1, const glm::vec2& a2, float bulgeA,
        const glm::vec2& b1, const glm::vec2& b2, float bulgeB);

    static std::vector<IntersectionPoint> PolylineIntersections(
        const std::vector<PolylineVertex>& polyA, bool closedA,
        const std::vector<PolylineVertex>& polyB, bool closedB);

    // Split a polyline at sorted intersection points.
    // sortedIntersections must be sorted by (segmentIndex, parameter).
    // Returns a list of sub-polyline vertex lists (all open).
    static std::vector<std::vector<PolylineVertex>> SplitPolyline(
        const std::vector<PolylineVertex>& poly, bool closed,
        const std::vector<IntersectionPoint>& sortedIntersections);
};