#include "Entities/Polyline.h"
#include "MathHelper.h"
#include <glm/ext/scalar_constants.hpp>
Polyline::Polyline(const DRW_LWPolyline& plydata)
{
	// store vertices with bulge information
	for (const auto& vertPtr : plydata.vertlist) {
		if (vertPtr) {
            m_plyvertices.emplace_back(
                static_cast<float>(vertPtr->x),
                static_cast<float>(vertPtr->y),
                static_cast<float>(vertPtr->bulge)
            );
		}
	}

	_closed = (plydata.flags & 1) != 0; // check if closed flag is set: 1 for closed polyline

	// Populate the base class vertices for OpenGL, handling bulge (arc) if present
	size_t vertCount = m_plyvertices.size();
	for (size_t i = 0; i < vertCount; ++i) {
		const auto& v1 = m_plyvertices[i];
		vertices.push_back(v1.position.x);
		vertices.push_back(v1.position.y);

		// Determine the next vertex index, considering closure
		size_t nextIdx = i + 1;
		bool hasNext = nextIdx < vertCount;
		if (!hasNext && _closed && vertCount > 1) {
			nextIdx = 0;
			hasNext = true;
		}

		// If bulge is non-zero and there is a next vertex, interpolate arc
		if (!MathHelper::IsZero(v1.bulge) && hasNext) {
			const auto& v2 = m_plyvertices[nextIdx];
			// Calculate arc points between v1 and v2 using bulge
			const int arcSegments = 16; // Number of segments for arc approximation
			glm::vec2 p1 = v1.position;
			glm::vec2 p2 = v2.position;
			double bulge = v1.bulge;

			// Center of the arc
			glm::vec2 center = MathHelper::CenterFromBulge(p1, p2, bulge);

			// Angles
			float angle1 = std::atan2(p1.y - center.y, p1.x - center.x);
			float angle2 = std::atan2(p2.y - center.y, p2.x - center.x);

			// Ensure correct direction
			if (bulge < 0.0f && angle2 > angle1) angle2 -= 2.0f * glm::pi<float>();
			if (bulge > 0.0f && angle2 < angle1) angle2 += 2.0f * glm::pi<float>();

			float radius = glm::distance(center, p1);
			for (int s = 1; s < arcSegments; ++s) {
				float t = static_cast<float>(s) / arcSegments;
				float theta = angle1 + t * (angle2 - angle1);
				float x = center.x + radius * std::cos(theta);
				float y = center.y + radius * std::sin(theta);
				vertices.push_back(x);
				vertices.push_back(y);
			}
		}
	}
}

void Polyline::draw(QOpenGLFunctions_3_3_Core* f) const
{
	if (vertices.empty() || !_vAO || !_vBO) return;

	f->glBindVertexArray(_vAO);
	f->glBindBuffer(GL_ARRAY_BUFFER, _vBO);

	// Draw as line strip or loop depending on _closed
	GLenum mode = _closed ? GL_LINE_LOOP : GL_LINE_STRIP;
	f->glDrawArrays(mode, 0, static_cast<GLsizei>(vertices.size() / 2));

	f->glBindBuffer(GL_ARRAY_BUFFER, 0);
	f->glBindVertexArray(0);
}