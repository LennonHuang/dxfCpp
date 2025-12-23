#pragma once

#include <string>
#include <QOpenGLFunctions_3_3_Core>

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
    void setColor(float r, float g, float b) {
        _color[0] = r;
        _color[1] = g;
        _color[2] = b;
    }

    // Buffer setup/teardown
    virtual void createBuffers(QOpenGLFunctions_3_3_Core* f);
    virtual void deleteBuffers(QOpenGLFunctions_3_3_Core* f);

protected:
    float _color[3] = { 1.0f, 1.0f, 1.0f };   // Default: white

    // OpenGL handles for a simple VAO + VBO
    GLuint _vAO = 0;
    GLuint _vBO = 0;

    // vertices for OpenGL to display
    std::vector<float> vertices;
};