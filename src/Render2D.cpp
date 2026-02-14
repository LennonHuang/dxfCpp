#include "Render2D.h"
#include <iostream>

static const char* vertexShaderSrc = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform mat4 uProjection;
uniform vec3 uColor;
out vec3 vColor;
out float vDist;
void main() {
    vColor = uColor;
    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);
}
)";

static const char* fragmentShaderSrc = R"(
#version 330 core
in vec3 vColor;
out vec4 FragColor;

uniform float alpha;

void main() {
    FragColor = vec4(vColor, alpha);
}
)";

Render2D::Render2D(int width, int height)
    : _width(width), _height(height), _shaderProgram(0),
    _camera((float)width, (float)height)
{
    // No GL calls here �� only in initGL.
}

Render2D::~Render2D()
{
    // Deletion of GL resources must be done with an active context.
}

void Render2D::addEntity(std::shared_ptr<Entity> entity)
{
    _entities.push_back(entity);
}

void Render2D::initGL(QOpenGLFunctions_3_3_Core* f)
{
    _shaderProgram = createShaderProgram(f, vertexShaderSrc, fragmentShaderSrc);

    f->glEnable(GL_BLEND);
    f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    f->glViewport(0, 0, _width, _height);

    setupProjection(f);

	// X and Y axes
    _xAxis = std::make_unique<Axis>(Axis::X, _height * 0.1f);
    _yAxis = std::make_unique<Axis>(Axis::Y, _height * 0.1f);
    _xAxis->createBuffers(f);
    _yAxis->createBuffers(f);
}

void Render2D::setupProjection(QOpenGLFunctions_3_3_Core* f)
{
    _projection = glm::ortho(
        0.0f, static_cast<float>(_width),
        0.0f, static_cast<float>(_height),
        -1.0f, 1.0f);

    f->glUseProgram(_shaderProgram);
    GLint projLoc = f->glGetUniformLocation(_shaderProgram, "uProjection");
    f->glUniformMatrix4fv(projLoc, 1, GL_FALSE, &_projection[0][0]);
}

void Render2D::render(QOpenGLFunctions_3_3_Core* f)
{
    f->glClear(GL_COLOR_BUFFER_BIT);

    f->glUseProgram(_shaderProgram);

    glm::mat4 viewProj = _camera.getMatrix();
    GLint loc = f->glGetUniformLocation(_shaderProgram, "uProjection");
    f->glUniformMatrix4fv(loc, 1, GL_FALSE, &viewProj[0][0]);

    GLint colorLoc = f->glGetUniformLocation(_shaderProgram, "uColor");

    for (auto& entity : _entities) {
        f->glUniform3fv(colorLoc, 1, entity->getColor());
		f->glUniform1f(f->glGetUniformLocation(_shaderProgram, "alpha"), entity->getAlpha());
        entity->draw(f);  // Entity::draw takes f
    }

	// Draw axes
    f->glUniform3fv(colorLoc, 1, _xAxis->getColor());
    _xAxis->draw(f);
    f->glUniform3fv(colorLoc, 1, _yAxis->getColor());
    _yAxis->draw(f);
}

void Render2D::resize(int width, int height, QOpenGLFunctions_3_3_Core* f)
{
    _width = width;
    _height = height;
    _camera.setViewport((float)width, (float)height);

    f->glViewport(0, 0, width, height);
    setupProjection(f);
}

void Render2D::handlePan(float dx, float dy)
{
    _camera.pan(dx, dy);
}

void Render2D::handleZoom(float delta, double mouseX, double mouseY)
{
    float factor = (delta > 0) ? 1.1f : 0.9f;
    glm::vec2 worldPos = screenToWorld(mouseX, mouseY);
    _camera.zoomAt(factor, worldPos);
}

GLuint Render2D::compileShader(QOpenGLFunctions_3_3_Core* f, GLenum type, const char* source)
{
    GLuint shader = f->glCreateShader(type);
    f->glShaderSource(shader, 1, &source, nullptr);
    f->glCompileShader(shader);

    GLint success;
    f->glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        f->glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compile error: " << infoLog << "\n";
    }
    return shader;
}

GLuint Render2D::createShaderProgram(QOpenGLFunctions_3_3_Core* f, const char* vertexSrc, const char* fragmentSrc)
{
    GLuint vs = compileShader(f, GL_VERTEX_SHADER, vertexSrc);
    GLuint fs = compileShader(f, GL_FRAGMENT_SHADER, fragmentSrc);

    GLuint prog = f->glCreateProgram();
    f->glAttachShader(prog, vs);
    f->glAttachShader(prog, fs);
    f->glLinkProgram(prog);

    GLint success;
    f->glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        f->glGetProgramInfoLog(prog, 512, nullptr, infoLog);
        std::cerr << "Program link error: " << infoLog << "\n";
    }

    f->glDeleteShader(vs);
    f->glDeleteShader(fs);

    return prog;
}

/// <summary>
/// Screen Coordinates (sx, sy) to World Coordinates (wx, wy)
/// </summary>
/// <param name="sx"></param>
/// <param name="sy"></param>
/// <returns></returns>
glm::vec2 Render2D::screenToWorld(double sx, double sy) const
{
    float wx = static_cast<float>(sx);
    float wy = static_cast<float>(_height - sy);
    return glm::vec2(wx, wy);
}

glm::vec2 Render2D::getMouseWorldPos(const QPoint& mousePos)
{
	auto worldPosInPixels = screenToWorld(mousePos.x(), mousePos.y());
    return (glm::vec2(worldPosInPixels.x, worldPosInPixels.y) - _camera.getOffset()) / static_cast<float>(_camera.getScale());
}

void Render2D::clearEntities(QOpenGLFunctions_3_3_Core* f) {
    for (auto& entity : _entities) {
        entity->deleteBuffers(f); // Free OpenGL resources
    }
    _entities.clear();
}

void Render2D::hightlightEntity(Entity* selectedEntity)
{
	for (auto& entity : _entities) {
		if (selectedEntity == nullptr || entity.get() == selectedEntity) {
			entity->setAlpha(1.0);
		}
		else {
            entity->setAlpha(0.2);
		}
	}
}

Entity* Render2D::pickEntity(const QPoint& screenPos)
{
    glm::vec2 worldPos = getMouseWorldPos(screenPos);
    // Tolerance in world units: ~5 pixels at current zoom
    float tolerance = 5.0f / static_cast<float>(_camera.getScale());

    for (auto& entity : _entities) {
        if (entity->hitTest(worldPos.x, worldPos.y, tolerance)) {
            return entity.get();
        }
    }
    return nullptr;
}
