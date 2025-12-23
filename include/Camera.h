#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera2D {
public:
    Camera2D(float width, float height)
        : _width(width), _height(height), _zoom(1.0f), _offset(0.0f, 0.0f) {
    }

    void setViewport(float width, float height) {
        _width = width;
        _height = height;
    }

    void zoomAt(float factor, glm::vec2 mousePos);
    void pan(float dx, float dy);
    void reset() { _zoom = 1.0f; _offset = glm::vec2(0.0f); }

    glm::mat4 getMatrix() const {
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(_offset, 0.0f));
        view = glm::scale(view, glm::vec3(_zoom, _zoom, 1.0f));

        // Orthographic projection ([-1, 1] space)
        //glm::mat4 proj = glm::ortho(-_width / _height, _width / _height, -1.0f, 1.0f);
        glm::mat4 proj = glm::ortho(0.0f, _width, 0.0f, _height);
        return proj * view;
    }

private:
    float _width, _height;
    float _zoom;
    glm::vec2 _offset;
};
