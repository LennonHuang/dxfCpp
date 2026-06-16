#include <glm/ext/scalar_constants.hpp>
#include "AutoDxfHelper.h"

static float PI = glm::pi<float>();
glm::vec2 AutoDxfHelper::CenterFromBulge(const glm::vec2& p1, const glm::vec2& p2, float bulge)
{
	if (IsZero(bulge)) {
		// No arc, just return midpoint
		return (p1 + p2) * 0.5f;
	}

	glm::vec2 chord = p2 - p1;
	float chordLen = glm::length(chord);

	// Distance from midpoint to center
	// derived from bulge definition
	float h = chordLen * (1 - bulge * bulge) / (4.0f * bulge);

	// Midpoint of the chord
	glm::vec2 mid = (p1 + p2) * 0.5f;

	// Perpendicular direction
	glm::vec2 perp(-chord.y, chord.x);
	perp = glm::normalize(perp);

	// Return center of the arc
	return mid + perp * h;
}


float AutoDxfHelper::RadiusFromBulge(const glm::vec2& p1, const glm::vec2& p2, float bulge)
{
	return glm::distance(CenterFromBulge(p1, p2, bulge), p1);
}

// Normalize `angle` into the half-open range [base, base + 2��).
static float NormalizeAngle(float angle, float base)
{
	while (angle < base)        angle += 2.f * PI;
	while (angle >= base + 2.f * PI) angle -= 2.f * PI;
	return angle;
}

bool AutoDxfHelper::AngleOnArc(float a1, float a2, float aQ, float bulge)
{
	if (bulge > 0.f) {
		// CCW: normalize a2 and aQ so they are >= a1
		a2 = NormalizeAngle(a2, a1);
		aQ = NormalizeAngle(aQ, a1);
		return aQ <= a2 + EPSILON;
	}
	else {
		// CW: normalize a2 and aQ so they are <= a1
		while (a2 > a1) a2 -= 2.f * PI;
		while (aQ > a1) aQ -= 2.f * PI;
		return aQ >= a2 - EPSILON;
	}
}

std::vector<AutoDxfHelper::IntersectionPoint> AutoDxfHelper::IntersectLineLine(
	const glm::vec2& p1, const glm::vec2& p2,
	const glm::vec2& q1, const glm::vec2& q2)
{
	glm::vec2 d1 = p2 - p1;
	glm::vec2 d2 = q2 - q1;
	glm::vec2 r = q1 - p1;

	float det = d1.x * d2.y - d1.y * d2.x;
	if (IsZero(det)) return {};   // parallel or collinear

	float t = (r.x * d2.y - r.y * d2.x) / det;
	float s = (r.x * d1.y - r.y * d1.x) / det;

	if (t < -EPSILON || t > 1.f + EPSILON) return {};
	if (s < -EPSILON || s > 1.f + EPSILON) return {};

	float tClamped = std::clamp(t, 0.f, 1.f);
	return { { p1 + tClamped * d1, -1, tClamped } }; // segment index -1 and will be assigned when processing polyline segments
}

std::vector<AutoDxfHelper::IntersectionPoint> AutoDxfHelper::IntersectLineArc(
	const glm::vec2& lp1, const glm::vec2& lp2,
	const glm::vec2& ap1, const glm::vec2& ap2, float bulge)
{
	glm::vec2 C = CenterFromBulge(ap1, ap2, bulge);
	float r = glm::distance(C, ap1);

	// P(t) = lp1 + t·d
	//  f = lp1 - C
	// substitute lp1:
	// |f + t·d|² = r²
	// (d·d)t² + 2(f·d)t + (f·f - r²) = 0
	glm::vec2 d = lp2 - lp1;
	glm::vec2 f = lp1 - C;

	float a = glm::dot(d, d);
	float b = 2.f * glm::dot(f, d);
	float c = glm::dot(f, f) - r * r;
	float disc = b * b - 4.f * a * c;

	if (disc < -EPSILON) return {};
	disc = std::max(0.f, disc);

	float sqrtDisc = std::sqrt(disc);
	float arcA1 = std::atan2(ap1.y - C.y, ap1.x - C.x);
	float arcA2 = std::atan2(ap2.y - C.y, ap2.x - C.x);

	std::vector<IntersectionPoint> result;

	for (int sign : { -1, 1 }) {
		float t = (-b + sign * sqrtDisc) / (2.f * a);
		if (t < -EPSILON || t > 1.f + EPSILON) continue;

		float tClamped = std::clamp(t, 0.f, 1.f);
		glm::vec2 pt = lp1 + tClamped * d;
		float aQ = std::atan2(pt.y - C.y, pt.x - C.x);

		if (AngleOnArc(arcA1, arcA2, aQ, bulge)) {
			result.push_back({ pt, -1, tClamped });
		}
	}

	// Collapse tangent duplicates
	if (result.size() == 2 && glm::distance(result[0].point, result[1].point) < EPSILON)
		return {};// ignore tangent intersection

	return result;
}

std::vector<AutoDxfHelper::IntersectionPoint> AutoDxfHelper::IntersectArcArc(
	const glm::vec2& a1, const glm::vec2& a2, float bulgeA,
	const glm::vec2& b1, const glm::vec2& b2, float bulgeB)
{
	glm::vec2 CA = CenterFromBulge(a1, a2, bulgeA);
	glm::vec2 CB = CenterFromBulge(b1, b2, bulgeB);
	float rA = glm::distance(CA, a1);
	float rB = glm::distance(CB, b1);

	glm::vec2 delta = CB - CA;
	float d = glm::length(delta);

	if (d < EPSILON)                       return {}; // concentric
	if (d > rA + rB + EPSILON)             return {}; // too far apart
	if (d < std::abs(rA - rB) - EPSILON)   return {}; // one inside the other

	float aVal = (rA * rA - rB * rB + d * d) / (2.f * d);
	float hSq = std::max(0.f, rA * rA - aVal * aVal);
	float h = std::sqrt(hSq);

	glm::vec2 mid = CA + (aVal / d) * delta;
	glm::vec2 perp = glm::vec2(-delta.y, delta.x) / d;

	glm::vec2 candidates[2] = { mid + h * perp, mid - h * perp };

	float aa1 = std::atan2(a1.y - CA.y, a1.x - CA.x);
	float aa2 = std::atan2(a2.y - CA.y, a2.x - CA.x);
	float ba1 = std::atan2(b1.y - CB.y, b1.x - CB.x);
	float ba2 = std::atan2(b2.y - CB.y, b2.x - CB.x);

	std::vector<IntersectionPoint> result;

	for (const auto& pt : candidates) {
		float aQA = std::atan2(pt.y - CA.y, pt.x - CA.x);
		float aQB = std::atan2(pt.y - CB.y, pt.x - CB.x);

		if (AngleOnArc(aa1, aa2, aQA, bulgeA) && AngleOnArc(ba1, ba2, aQB, bulgeB)) {
			auto [bulge1, bulge2] = SplitBulge(a1, a2, bulgeA, pt);
			float param = bulge1 / bulgeA;
			result.push_back({ pt, -1, param });
		}
	}

	if (result.size() == 2 && glm::distance(result[0].point, result[1].point) < EPSILON)
		return {};// ignore tangent intersection

	return result;
}

std::pair<float, float> AutoDxfHelper::SplitBulge(
	const glm::vec2& p1, const glm::vec2& p2, float bulge,
	const glm::vec2& splitPoint)
{
	glm::vec2 C = CenterFromBulge(p1, p2, bulge);
	float a1 = std::atan2(p1.y - C.y, p1.x - C.x);
	float aQ = std::atan2(splitPoint.y - C.y, splitPoint.x - C.x);

	float totalAngle = 4.f * std::atan(bulge);   // signed total included angle

	// Normalize aQ relative to a1, matching the sweep direction
	float theta1;
	if (bulge > 0.f) {
		aQ = NormalizeAngle(aQ, a1);   // aQ >= a1
		theta1 = aQ - a1;                  // positive
	}
	else {
		while (aQ > a1) aQ -= 2.f * PI;   // aQ <= a1
		theta1 = aQ - a1;                  // negative
	}

	float theta2 = totalAngle - theta1;

	return { std::tan(theta1 / 4.f), std::tan(theta2 / 4.f) };
}

std::vector<AutoDxfHelper::IntersectionPoint> AutoDxfHelper::IntersectSegments(
	const PolylineSegment& a, const PolylineSegment& b)
{
	if (!a.isArc() && !b.isArc()) {
		return IntersectLineLine(a.start, a.end, b.start, b.end);
	}
	else if (!a.isArc() && b.isArc()) {
		return IntersectLineArc(a.start, a.end, b.start, b.end, b.bulge);
	}
	else if (a.isArc() && !b.isArc()) {
		auto hits = IntersectLineArc(b.start, b.end, a.start, a.end, a.bulge);
		// IntersectLineArc returns parameter along the line (b = trimline).
		// Recompute parameter along the arc (a = ogPly) using SplitBulge.
		for (auto& ip : hits) {
			auto [bulge1, bulge2] = SplitBulge(a.start, a.end, a.bulge, ip.point);
			ip.parameter = bulge1 / a.bulge;
		}
		return hits;
	}
	else {
		return IntersectArcArc(a.start, a.end, a.bulge, b.start, b.end, b.bulge);
	}
}

std::vector<std::vector<PolylineVertex>> AutoDxfHelper::SplitPolyline(
	const std::vector<PolylineVertex>& poly, bool closed,
	const std::vector<IntersectionPoint>& sortedIntersections)
{
	std::vector<std::vector<PolylineVertex>> result;
	std::vector<PolylineVertex> current;

	size_t vertCount = poly.size();
	size_t segCount = closed ? vertCount : vertCount - 1;
	size_t ipIdx = 0;

	for (size_t i = 0; i < segCount; ++i) {
		size_t nextI = (i + 1) % vertCount;
		glm::vec2 segEnd = poly[nextI].position;

		glm::vec2 curPos = poly[i].position;
		float curBulge = poly[i].bulge;

		while (ipIdx < sortedIntersections.size() &&
			sortedIntersections[ipIdx].segmentIndex == static_cast<int>(i)) {

			glm::vec2 splitPt = sortedIntersections[ipIdx].point;

			float b1 = 0.f, b2 = 0.f;
			if (!IsZero(curBulge)) {
				auto [bulge1, bulge2] = SplitBulge(curPos, segEnd, curBulge, splitPt);
				b1 = bulge1;
				b2 = bulge2;
			}

			// Close current sub-poly: curPos --b1--> splitPt
			current.emplace_back(curPos.x, curPos.y, b1);
			current.emplace_back(splitPt.x, splitPt.y, 0.f);
			result.push_back(std::move(current));
			current.clear();

			curPos = splitPt;
			curBulge = b2;
			++ipIdx;
		}

		// Add remaining part of this segment to current sub-poly
		current.emplace_back(curPos.x, curPos.y, curBulge);
	}

	if (!closed && vertCount > 0) {
		// Open polyline: add the final endpoint
		current.emplace_back(poly.back().position.x, poly.back().position.y, 0.f);
	}
	else if (closed && !result.empty() && !current.empty()) {
		// Closed polyline with intersections: the last sub-poly wraps around
		// to the first. Merge them into one continuous open polyline.
		auto& firstSub = result.front();
		for (const auto& v : firstSub) {
			current.push_back(v);
		}
		result.erase(result.begin());
	}
	else if (closed && result.empty() && vertCount > 0) {
		// Closed polyline with no intersections: add the closing vertex
		current.emplace_back(poly[0].position.x, poly[0].position.y, 0.f);
	}

	if (!current.empty()) {
		result.push_back(std::move(current));
	}

	return result;
}

std::vector<AutoDxfHelper::IntersectionPoint> AutoDxfHelper::PolylineIntersections(
	const std::vector<PolylineVertex>& polyA, bool closedA,
	const std::vector<PolylineVertex>& polyB, bool closedB)
{
	std::vector<IntersectionPoint> result;

	auto segCount = [](size_t vertCount, bool closed) -> size_t {
		if (vertCount < 2) return 0;
		return closed ? vertCount : vertCount - 1;
	};

	size_t segsA = segCount(polyA.size(), closedA);
	size_t segsB = segCount(polyB.size(), closedB);

	for (size_t i = 0; i < segsA; ++i) {
		size_t nextI = (i + 1) % polyA.size();
		PolylineSegment segA{ polyA[i].position, polyA[nextI].position,
			polyA[i].bulge, static_cast<int>(i) };

		for (size_t j = 0; j < segsB; ++j) {
			size_t nextJ = (j + 1) % polyB.size();
			PolylineSegment segB{ polyB[j].position, polyB[nextJ].position,
				polyB[j].bulge, static_cast<int>(j) };

			auto hits = IntersectSegments(segA, segB);

			for (auto& ip : hits) {
				ip.segmentIndex = static_cast<int>(i);
				result.push_back(ip);
			}
		}
	}

	return result;
}
