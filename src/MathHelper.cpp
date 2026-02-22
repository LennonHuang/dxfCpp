#include "MathHelper.h"
#include <algorithm>
#include <cmath>

static constexpr float PI      = glm::pi<float>();
static constexpr float TWO_PI  = 2.0f * PI;
static constexpr float EPS     = MathHelper::EPSILON;

// ---------------------------------------------------------------------------
// Helper: parametric position of an angle on an arc.
// Returns t in [0,1] if the angle lies on the arc, or -1 if it doesn't.
// ---------------------------------------------------------------------------
static float arcParam(float angle, float startAngle, float sweep)
{
    float delta = angle - startAngle;

    // Normalise delta into the sweep direction
    if (sweep > 0) {
        while (delta < -EPS)        delta += TWO_PI;
        while (delta > TWO_PI + EPS) delta -= TWO_PI;
    } else {
        while (delta > EPS)          delta -= TWO_PI;
        while (delta < -TWO_PI - EPS) delta += TWO_PI;
    }

    float t = delta / sweep;
    if (t < -EPS || t > 1.0f + EPS)
        return -1.0f;
    return std::clamp(t, 0.0f, 1.0f);
}

// ---------------------------------------------------------------------------
// BuildSegments
// ---------------------------------------------------------------------------
std::vector<PolylineSegment> MathHelper::BuildSegments(
    const std::vector<PolylineVertex>& verts, bool closed)
{
    std::vector<PolylineSegment> segs;
    size_t n = verts.size();
    if (n < 2) return segs;

    size_t count = closed ? n : n - 1;
    segs.reserve(count);

    for (size_t i = 0; i < count; ++i) {
        size_t j = (i + 1) % n;
        const auto& v1 = verts[i];
        const auto& v2 = verts[j];

        PolylineSegment seg;
        seg.start = v1.position;
        seg.end   = v2.position;

        if (IsZero(v1.bulge)) {
            seg.isArc      = false;
            seg.center     = {};
            seg.radius     = 0.0f;
            seg.startAngle = 0.0f;
            seg.endAngle   = 0.0f;
        } else {
            seg.isArc  = true;
            seg.center = CenterFromBulge(v1.position, v2.position, v1.bulge);
            seg.radius = glm::distance(seg.center, v1.position);
            seg.startAngle = std::atan2(v1.position.y - seg.center.y,
                                        v1.position.x - seg.center.x);
            seg.endAngle   = std::atan2(v2.position.y - seg.center.y,
                                        v2.position.x - seg.center.x);

            // Ensure correct sweep direction (same logic as Polyline constructor)
            if (v1.bulge < 0.0f && seg.endAngle > seg.startAngle)
                seg.endAngle -= TWO_PI;
            if (v1.bulge > 0.0f && seg.endAngle < seg.startAngle)
                seg.endAngle += TWO_PI;
        }

        segs.push_back(seg);
    }
    return segs;
}

// ---------------------------------------------------------------------------
// SegmentIntersect  (line vs line)
// ---------------------------------------------------------------------------
bool MathHelper::SegmentIntersect(
    const glm::vec2& a1, const glm::vec2& a2,
    const glm::vec2& b1, const glm::vec2& b2,
    float& tA, float& tB, glm::vec2& point)
{
    glm::vec2 dA = a2 - a1;
    glm::vec2 dB = b2 - b1;

    float cross = dA.x * dB.y - dA.y * dB.x;

    if (std::abs(cross) < EPSILON)
        return false;

    glm::vec2 diff = b1 - a1;
    tA = (diff.x * dB.y - diff.y * dB.x) / cross;
    tB = (diff.x * dA.y - diff.y * dA.x) / cross;

    if (tA < -EPSILON || tA > 1.0f + EPSILON ||
        tB < -EPSILON || tB > 1.0f + EPSILON)
        return false;

    tA = std::clamp(tA, 0.0f, 1.0f);
    tB = std::clamp(tB, 0.0f, 1.0f);

    point = a1 + tA * dA;
    return true;
}

// ---------------------------------------------------------------------------
// Arc vs Line intersection helpers
// ---------------------------------------------------------------------------

// Find intersections between an arc and a line segment.
// Pushes results into 'out'. segIdxA is for the arc, segIdxB for the line.
static void arcLineIntersect(
    const PolylineSegment& arc, int segIdxA,
    const PolylineSegment& line, int segIdxB,
    std::vector<IntersectResult>& out)
{
    // Line: P = P1 + t * D,  t in [0,1]
    glm::vec2 P1 = line.start;
    glm::vec2 D  = line.end - line.start;

    // Substitute into circle equation |P - C|^2 = r^2
    // let E = P1 - C
    glm::vec2 E = P1 - arc.center;
    float a = glm::dot(D, D);
    float b = 2.0f * glm::dot(E, D);
    float c = glm::dot(E, E) - arc.radius * arc.radius;

    float disc = b * b - 4.0f * a * c;
    if (disc < 0.0f)
        return;

    float sqrtDisc = std::sqrt(disc);
    float inv2a    = 1.0f / (2.0f * a);

    float sweep = arc.endAngle - arc.startAngle;

    for (int sign = -1; sign <= 1; sign += 2) {
        float tLine = (-b + sign * sqrtDisc) * inv2a;

        // Must be on the line segment
        if (tLine < -EPS || tLine > 1.0f + EPS)
            continue;
        tLine = std::clamp(tLine, 0.0f, 1.0f);

        glm::vec2 pt = P1 + tLine * D;

        // Check if the point is on the arc's angular span
        float angle = std::atan2(pt.y - arc.center.y, pt.x - arc.center.x);
        float tArc  = arcParam(angle, arc.startAngle, sweep);
        if (tArc < 0.0f)
            continue;

        out.push_back({ pt, segIdxA, tArc, segIdxB, tLine });
    }
}

// ---------------------------------------------------------------------------
// Arc vs Arc intersection
// ---------------------------------------------------------------------------
static void arcArcIntersect(
    const PolylineSegment& arcA, int segIdxA,
    const PolylineSegment& arcB, int segIdxB,
    std::vector<IntersectResult>& out)
{
    glm::vec2 delta = arcB.center - arcA.center;
    float d = glm::length(delta);

    // No intersection if circles are too far apart, nested, or concentric
    if (d < EPS)
        return;
    if (d > arcA.radius + arcB.radius + EPS)
        return;
    if (d < std::abs(arcA.radius - arcB.radius) - EPS)
        return;

    // Distance from arcA center to the line connecting intersection points
    float a = (arcA.radius * arcA.radius - arcB.radius * arcB.radius + d * d) / (2.0f * d);
    float h2 = arcA.radius * arcA.radius - a * a;
    if (h2 < 0.0f) h2 = 0.0f;
    float h = std::sqrt(h2);

    // Midpoint on the line between the two intersection points
    glm::vec2 mid = arcA.center + (a / d) * delta;

    // Perpendicular direction
    glm::vec2 perp(-delta.y / d, delta.x / d);

    float sweepA = arcA.endAngle - arcA.startAngle;
    float sweepB = arcB.endAngle - arcB.startAngle;

    // Check up to 2 candidate points
    int numPoints = (h < EPS) ? 1 : 2;
    for (int k = 0; k < numPoints; ++k) {
        glm::vec2 pt = (k == 0) ? mid + h * perp : mid - h * perp;

        // Check if pt lies on both arcs
        float angA = std::atan2(pt.y - arcA.center.y, pt.x - arcA.center.x);
        float tA   = arcParam(angA, arcA.startAngle, sweepA);
        if (tA < 0.0f) continue;

        float angB = std::atan2(pt.y - arcB.center.y, pt.x - arcB.center.x);
        float tB   = arcParam(angB, arcB.startAngle, sweepB);
        if (tB < 0.0f) continue;

        out.push_back({ pt, segIdxA, tA, segIdxB, tB });
    }
}

// ---------------------------------------------------------------------------
// Polyline intersection — flat vertex arrays (line segments only, original)
// ---------------------------------------------------------------------------
std::vector<IntersectResult> MathHelper::PolylineIntersect(
    const std::vector<float>& vertsA, bool closedA,
    const std::vector<float>& vertsB, bool closedB)
{
    std::vector<IntersectResult> results;

    size_t countA = vertsA.size() / 2;
    size_t countB = vertsB.size() / 2;
    if (countA < 2 || countB < 2)
        return results;

    size_t segsA = closedA ? countA : countA - 1;
    size_t segsB = closedB ? countB : countB - 1;

    for (size_t i = 0; i < segsA; ++i) {
        size_t i2 = (i + 1) % countA;
        glm::vec2 a1(vertsA[i  * 2], vertsA[i  * 2 + 1]);
        glm::vec2 a2(vertsA[i2 * 2], vertsA[i2 * 2 + 1]);

        for (size_t j = 0; j < segsB; ++j) {
            size_t j2 = (j + 1) % countB;
            glm::vec2 b1(vertsB[j  * 2], vertsB[j  * 2 + 1]);
            glm::vec2 b2(vertsB[j2 * 2], vertsB[j2 * 2 + 1]);

            float tA, tB;
            glm::vec2 pt;
            if (SegmentIntersect(a1, a2, b1, b2, tA, tB, pt)) {
                results.push_back({
                    pt,
                    static_cast<int>(i), tA,
                    static_cast<int>(j), tB
                });
            }
        }
    }

    std::sort(results.begin(), results.end(),
        [](const IntersectResult& a, const IntersectResult& b) {
            if (a.segIndexA != b.segIndexA)
                return a.segIndexA < b.segIndexA;
            return a.paramA < b.paramA;
        });

    return results;
}

// ---------------------------------------------------------------------------
// Polyline intersection — arc-aware (uses PolylineSegment lists)
// ---------------------------------------------------------------------------
std::vector<IntersectResult> MathHelper::PolylineIntersect(
    const std::vector<PolylineSegment>& segsA,
    const std::vector<PolylineSegment>& segsB)
{
    std::vector<IntersectResult> results;

    for (int i = 0; i < static_cast<int>(segsA.size()); ++i) {
        const auto& sA = segsA[i];

        for (int j = 0; j < static_cast<int>(segsB.size()); ++j) {
            const auto& sB = segsB[j];

            if (!sA.isArc && !sB.isArc) {
                // line vs line
                float tA, tB;
                glm::vec2 pt;
                if (SegmentIntersect(sA.start, sA.end, sB.start, sB.end, tA, tB, pt))
                    results.push_back({ pt, i, tA, j, tB });
            }
            else if (sA.isArc && !sB.isArc) {
                // arc A vs line B
                arcLineIntersect(sA, i, sB, j, results);
            }
            else if (!sA.isArc && sB.isArc) {
                // line A vs arc B — need results indexed as A/B
                // arcLineIntersect treats first arg as arc.
                // We call it with arc=sB, line=sA, then swap indices.
                std::vector<IntersectResult> tmp;
                arcLineIntersect(sB, j, sA, i, tmp);
                for (auto& r : tmp)
                    results.push_back({ r.point, r.segIndexB, r.paramB, r.segIndexA, r.paramA });
            }
            else {
                // arc vs arc
                arcArcIntersect(sA, i, sB, j, results);
            }
        }
    }

    // Sort by vertex order along polyline A
    std::sort(results.begin(), results.end(),
        [](const IntersectResult& a, const IntersectResult& b) {
            if (a.segIndexA != b.segIndexA)
                return a.segIndexA < b.segIndexA;
            return a.paramA < b.paramA;
        });

    return results;
}
