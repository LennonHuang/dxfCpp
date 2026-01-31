#pragma once  
#include <QOpenGLWidget>  
#include <QOpenGLFunctions_3_3_Core>  
#include <QOpenGLShaderProgram>  
#include <Render2D.h>  
#include "Dxfloader.h"  
#include <QMouseEvent>  
#include <QWheelEvent>  

class MyQOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core  
{  
   Q_OBJECT  
public:  
   explicit MyQOpenGLWidget(QWidget* parent = nullptr);  
   ~MyQOpenGLWidget() override;  

   // Add the missing method declaration  
   void loadDxf(const QString& fileName);

public slots:
	void OnClearDxf();
signals:
	void MouseMoved(const QPointF&);

protected:  
   void initializeGL() override;  
   void resizeGL(int w, int h) override;  
   void paintGL() override;  

   // Mouse event handlers for zoom and pan  
   void wheelEvent(QWheelEvent* event) override;  
   void mousePressEvent(QMouseEvent* event) override;  
   void mouseMoveEvent(QMouseEvent* event) override;  
   void mouseReleaseEvent(QMouseEvent* event) override;



private:  
   Render2D* m_renderer;  

   QPoint m_lastMousePos;  
   bool m_panning = false;  
};
