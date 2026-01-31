#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_AutoDxfCpp.h"
#include "myqopenglwidget.h"

class AutoDxfCpp : public QMainWindow
{
	Q_OBJECT

public:
	AutoDxfCpp(QWidget* parent = nullptr);
	~AutoDxfCpp();

private:
	Ui::AutoDxfCppClass ui;
	MyQOpenGLWidget* m_oglWidget;

private slots:
	void OnLoadDxf();
	void OnMouseMoved(const QPointF& pos);
};