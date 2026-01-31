#include "myqopenglwidget.h"
#include <QOpenGLContext>
#include <QOpenGLVersionFunctionsFactory>

MyQOpenGLWidget::MyQOpenGLWidget(QWidget* parent)
    : QOpenGLWidget(parent),
    m_renderer(nullptr)
{
    setMouseTracking(true);
}

MyQOpenGLWidget::~MyQOpenGLWidget()
{
    makeCurrent();

    delete m_renderer;
    m_renderer = nullptr;
    doneCurrent();
}

void MyQOpenGLWidget::initializeGL()
{
    // Ask for the 3.3 Core functions for the current context
    auto* f = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(context());
    if (!f) {
        qFatal("Failed to get OpenGL 3.3 core functions");
        return;
    }

    // Initialize GL function pointers for this context
    f->initializeOpenGLFunctions();

    // Create and initialize renderer
    m_renderer = new Render2D(width(), height());
    m_renderer->initGL(f);
    m_renderer->setupProjection(f);
}

void MyQOpenGLWidget::resizeGL(int w, int h)
{
    auto* f = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(context());
    if (!f) return;

    f->glViewport(0, 0, w, h);

    if (m_renderer) {
        m_renderer->resize(w, h, f);
    }
}

void MyQOpenGLWidget::paintGL()
{
    auto* f = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(context());
    if (!f) return;

    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (m_renderer) {
        m_renderer->render(f);
    }
}

void MyQOpenGLWidget::loadDxf(const QString& fileName)
{
	OnClearDxf(); // Clear existing entities
    if (!m_renderer)
    {
        return;
    }

    makeCurrent();  // IMPORTANT: ensure GL context is current

    // Ask for the 3.3 Core functions for the current context
    auto* f = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(context());
    if (!f) {
        qFatal("Failed to get OpenGL 3.3 core functions");
        return;
    }

    DxfLoader loader;
    std::string path = fileName.toLocal8Bit().constData(); // Window Chinese Character Friendly 
    if (loader.load(path)) {
        for (auto& entity : loader.getEntities()) {
            entity->createBuffers(f);
            m_renderer->addEntity(entity);
        }

        // update draw
		update();
    }

    doneCurrent();  // release context
}

void MyQOpenGLWidget::wheelEvent(QWheelEvent* event)
{
    if (!m_renderer) return;

    // Get mouse position
    QPoint pos = event->position().toPoint();

    // Determine zoom direction
    float delta = event->angleDelta().y();

    // Call renderer's zoom handler
    auto* f = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(context());
    if (f) {
        m_renderer->handleZoom(delta, pos.x(), pos.y());
        update();  // Trigger repaint
    }

    event->accept();
}

void MyQOpenGLWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton) {
        m_lastMousePos = event->pos();
        m_panning = true;
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
    else {
        event->ignore();
    }
}

void MyQOpenGLWidget::mouseMoveEvent(QMouseEvent* event)
{
    QPoint currentPos = event->pos();
    // Update Position on MainWindow Status Bar
	glm::vec2 wpos = m_renderer->getMouseWorldPos(currentPos);
    emit MouseMoved(QPointF(wpos.x,wpos.y));

	// Handle panning
    if (m_panning && m_renderer) {
        QPoint delta = currentPos - m_lastMousePos;

        // Convert to pan amount (you might want to adjust the sensitivity)
        float dx = static_cast<float>(delta.x());
        float dy = static_cast<float>(-delta.y());  // Invert Y for natural panning

        // Call renderer's pan handler
        m_renderer->handlePan(dx, dy);

        m_lastMousePos = currentPos;
        update();  // Trigger repaint
        event->accept();
    }
    else {
        event->ignore();
    }
}

void MyQOpenGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::MiddleButton) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    }
    else {
        event->ignore();
    }
}

void MyQOpenGLWidget::OnClearDxf()
{
    if (!m_renderer)
        return;

    makeCurrent(); // Ensure OpenGL context is current

    auto* f = QOpenGLVersionFunctionsFactory::get<QOpenGLFunctions_3_3_Core>(context());
    if (f) {
        m_renderer->clearEntities(f);
        update(); // Trigger repaint
    }

    doneCurrent();
}

