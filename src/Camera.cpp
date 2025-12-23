#include <Camera.h>
void Camera2D::pan(float dx, float dy) {
    _offset += glm::vec2(dx, dy);
}

void Camera2D::zoomAt(float factor, glm::vec2 mousePos) {
    // mousePos = position in world coordinates
    _offset = mousePos - (mousePos - _offset) * factor;
    _zoom *= factor;
}
