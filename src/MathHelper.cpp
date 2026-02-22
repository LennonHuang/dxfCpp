#include "MathHelper.h"
#include <algorithm>

bool MathHelper::SegmentIntersect(
    const glm::vec2& a1, const glm::vec2& a2,
    const glm::vec2& b1, const glm::vec2& b2,
    float& tA, float& tB, glm::vec2& point)
{
    // Direction vectors
    glm::vec2 dA = a2 - a1;
    glm::vec2 dB = b2 - b1;

    // Cross product (2D determinant)
    float cross = dA.x * dB.y - dA.y * dB.x;

    // Parallel or degenerate — no single intersection point
    if (std::abs(cross) < EPSILON)
        return false;

    // Solve for parametric positions:
    //   a1 + tA * dA = b1 + tB * dB
    //   (b1 - a1) = tA * dA - tB * dB
    glm::vec2 diff = b1 - a1;
    tA = (diff.x * dB.y - diff.y * dB.x) / cross;
    tB = (diff.x * dA.y - diff.y * dA.x) / cross;

    // Both parameters must be within [0, 1] for a segment intersection
    if (tA < -EPSILON || tA > 1.0f + EPSILON ||
        tB < -EPSILON || tB > 1.0f + EPSILON)
        return false;

    // Clamp to exact [0,1] to avoid floating-point overshoot
    tA = std::clamp(tA, 0.0f, 1.0f);
    tB = std::clamp(tB, 0.0f, 1.0f);

    point = a1 + tA * dA;
    return true;
}

std::vector<IntersectResult> MathHelper::PolylineIntersect(
    const std::vector<float>& vertsA, bool closedA,
    const std::vector<float>& vertsB, bool closedB)
{
    std::vector<IntersectResult> results;

    size_t countA = vertsA.size() / 2;
    size_t countB = vertsB.size() / 2;
    if (countA < 2 || countB < 2)
        return results;

    // Number of segments in each polyline
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

    // Sort by vertex order along polyline A (segment index first, then parametric t)
    std::sort(results.begin(), results.end(),
        [](const IntersectResult& a, const IntersectResult& b) {
            if (a.segIndexA != b.segIndexA)
                return a.segIndexA < b.segIndexA;
            return a.paramA < b.paramA;
        });

    return results;
}
