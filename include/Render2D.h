#pragma once

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Include Qt¡¯s OpenGL 3.3 core functions
#include <QOpenGLFunctions_3_3_Core>
#include <QPoint>
#include "Entities/Entity.h"
#include "Camera.h"
#include "Entities/Axis.h"

class Render2D
{
public:
    Render2D(int width, int height);
    ~Render2D();

    void addEntity(std::shared_ptr<Entity> entity);

    // All methods that call OpenGL take a QOpenGLFunctions_3_3_Core*,
    // which must be obtained from the current context.
    void initGL(QOpenGLFunctions_3_3_Core* f);
    void setupProjection(QOpenGLFunctions_3_3_Core* f);
    void render(QOpenGLFunctions_3_3_Core* f);
    void resize(int width, int height, QOpenGLFunctions_3_3_Core* f);

    void clearEntities(QOpenGLFunctions_3_3_Core* f);

    void handlePan(float dx, float dy);
    void handleZoom(float delta, double mouseX, double mouseY);

	// from mouse position to world position
	glm::vec2 getMouseWorldPos(const QPoint&);

private:
    GLuint compileShader(QOpenGLFunctions_3_3_Core* f, GLenum type, const char* source);
    GLuint createShaderProgram(QOpenGLFunctions_3_3_Core* f, const char* vertexSrc, const char* fragmentSrc);

    glm::vec2 screenToWorld(double sx, double sy) const;

    std::unique_ptr<Axis> _xAxis, _yAxis;

private:
    int _width;
    int _height;
    GLuint _shaderProgram;
    glm::mat4 _projection;

    Camera2D _camera;
    std::vector<std::shared_ptr<Entity>> _entities;
};