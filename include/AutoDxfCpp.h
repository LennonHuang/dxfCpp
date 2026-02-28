#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_AutoDxfCpp.h"
#include "myqopenglwidget.h"

class AutoDxfCpp : public QMainWindow
{
	Q_OBJECT

public:
	AutoDxfCpp(QWidget* parent = nullptr, bool showFileMenu = true);
	~AutoDxfCpp();

private:
	Ui::AutoDxfCppClass ui;
	MyQOpenGLWidget* m_oglWidget;

private slots:
	void OnLoadDxf();
	void OnNewViewer();
	void OnMouseMoved(const QPointF& pos);
	void OnUpdateTreeModel(QStandardItemModel* model);
	void onTreeItemClicked(const QModelIndex& index);
	void onEntitySelectedInViewport(Entity* entity);
};