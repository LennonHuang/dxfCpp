#include <glm/glm.hpp>
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
};