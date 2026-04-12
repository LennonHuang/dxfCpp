#pragma once

#include <glm/glm.hpp>
#include <cmath>
#include <optional>
#include <vector>

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

    // 2D cross product (scalar): a.x*b.y - a.y*b.x
    static float cross2D(const glm::vec2& a, const glm::vec2& b) {
        return a.x * b.y - a.y * b.x;
    }

    // Infinite line-line intersection.
    // Lines are defined by p1->p2 and p3->p4.
    // Returns the intersection point, or nullopt if lines are parallel/coincident.
    static std::optional<glm::vec2> lineLineIntersect(
        const glm::vec2& p1, const glm::vec2& p2,
        const glm::vec2& p3, const glm::vec2& p4)
    {
        glm::vec2 d1 = p2 - p1;
        glm::vec2 d2 = p4 - p3;
        float denom = cross2D(d1, d2);
        if (IsZero(denom))
            return std::nullopt; // parallel or coincident
        float t = cross2D(p3 - p1, d2) / denom;
        return p1 + t * d1;
    }

    // Segment-segment intersection (bounded to [0,1] on both segments).
    // Returns the intersection point, or nullopt if segments do not intersect.
    static std::optional<glm::vec2> segmentSegmentIntersect(
        const glm::vec2& p1, const glm::vec2& p2,
        const glm::vec2& p3, const glm::vec2& p4)
    {
        glm::vec2 d1 = p2 - p1;
        glm::vec2 d2 = p4 - p3;
        float denom = cross2D(d1, d2);
        if (IsZero(denom))
            return std::nullopt; // parallel
        glm::vec2 diff = p3 - p1;
        float t = cross2D(diff, d2) / denom;
        float s = cross2D(diff, d1) / denom;
        if (t < 0.f || t > 1.f || s < 0.f || s > 1.f)
            return std::nullopt;
        return p1 + t * d1;
    }

    // Infinite line-circle intersection.
    // Line is defined by p1->p2 (extended infinitely).
    // Returns 0, 1, or 2 intersection points.
    static std::vector<glm::vec2> lineCircleIntersect(
        const glm::vec2& p1, const glm::vec2& p2,
        const glm::vec2& center, float radius)
    {
        std::vector<glm::vec2> result;
        glm::vec2 d = p2 - p1;
        glm::vec2 f = p1 - center;
        float a = glm::dot(d, d);
        float b = 2.0f * glm::dot(f, d);
        float c = glm::dot(f, f) - radius * radius;
        float disc = b * b - 4.0f * a * c;
        if (disc < 0.f) return result;
        float sqrtDisc = std::sqrt(disc);
        float t1 = (-b - sqrtDisc) / (2.0f * a);
        float t2 = (-b + sqrtDisc) / (2.0f * a);
        result.push_back(p1 + t1 * d);
        if (!IsZero(disc))
            result.push_back(p1 + t2 * d);
        return result;
    }

    // Segment-circle intersection (bounded to [0,1] on the segment).
    // Returns 0, 1, or 2 intersection points that lie on the segment.
    static std::vector<glm::vec2> segmentCircleIntersect(
        const glm::vec2& p1, const glm::vec2& p2,
        const glm::vec2& center, float radius)
    {
        std::vector<glm::vec2> result;
        glm::vec2 d = p2 - p1;
        glm::vec2 f = p1 - center;
        float a = glm::dot(d, d);
        float b = 2.0f * glm::dot(f, d);
        float c = glm::dot(f, f) - radius * radius;
        float disc = b * b - 4.0f * a * c;
        if (disc < 0.f) return result;
        float sqrtDisc = std::sqrt(std::max(0.f, disc));
        float t1 = (-b - sqrtDisc) / (2.0f * a);
        float t2 = (-b + sqrtDisc) / (2.0f * a);
        if (t1 >= 0.f && t1 <= 1.f) result.push_back(p1 + t1 * d);
        if (!IsZero(t2 - t1) && t2 >= 0.f && t2 <= 1.f) result.push_back(p1 + t2 * d);
        return result;
    }

    // Circle-circle intersection.
    // Returns 0, 1, or 2 intersection points.
    static std::vector<glm::vec2> circleCircleIntersect(
        const glm::vec2& c1, float r1,
        const glm::vec2& c2, float r2)
    {
        std::vector<glm::vec2> result;
        glm::vec2 diff = c2 - c1;
        float d = glm::length(diff);
        if (IsZero(d)) return result;                        // concentric
        if (d > r1 + r2 + EPSILON) return result;            // too far apart
        if (d < std::abs(r1 - r2) - EPSILON) return result; // one inside the other
        float a = (r1 * r1 - r2 * r2 + d * d) / (2.0f * d);
        float h2 = r1 * r1 - a * a;
        if (h2 < 0.f) return result;
        float h = std::sqrt(h2);
        glm::vec2 mid = c1 + (a / d) * diff;
        glm::vec2 perp(-diff.y / d, diff.x / d);
        result.push_back(mid + h * perp);
        if (!IsZero(h))
            result.push_back(mid - h * perp);
        return result;
    }
};
