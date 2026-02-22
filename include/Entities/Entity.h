#pragma once

#include <string>
#include <QOpenGLFunctions_3_3_Core>
#include <glm/vec2.hpp>

class Entity
{
public:
    Entity();
    virtual ~Entity() = default;

    // Draw with the given OpenGL function resolver.
    // Derived classes must implement this.
    virtual void draw(QOpenGLFunctions_3_3_Core* f) const = 0;

    // Optional type identification
    virtual std::string getType() const { return "Entity"; }

    // Color getters/setters
    const float* getColor() const { return _color; }
    float getAlpha() const { return _alpha; }
    void setColor(float r, float g, float b) {
        _color[0] = r;
        _color[1] = g;
        _color[2] = b;
    }
	void setAlpha(float alpha) {
		_alpha = alpha;
	}

    // Access to geometry data
    const std::vector<float>& getVertices() const { return vertices; }
    bool isClosed() const { return _closed; }

    // Hit test: returns true if worldPos is within tolerance of this entity's geometry
    virtual bool hitTest(float wx, float wy, float tolerance) const;

    // Buffer setup/teardown
    virtual void createBuffers(QOpenGLFunctions_3_3_Core* f);
    virtual void deleteBuffers(QOpenGLFunctions_3_3_Core* f);

protected:
    float _color[3] = { 1.0f, 1.0f, 1.0f };   // Default: white
	float _alpha = 1.0f; // Default: fully opaque
    bool _closed = false; // Whether the shape is closed (loop vs strip)

    // OpenGL handles for a simple VAO + VBO
    GLuint _vAO = 0;
    GLuint _vBO = 0;

    // vertices for OpenGL to display
    std::vector<float> vertices;
};