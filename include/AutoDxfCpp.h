#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_AutoDxfCpp.h"
#include "myqopenglwidget.h"

class AutoDxfCpp : public QMainWindow
{
	Q_OBJECT

public:
	AutoDxfCpp(bool showMenu = true, QWidget* parent = nullptr);
	~AutoDxfCpp();

	MyQOpenGLWidget* getOglWidget() const { return m_oglWidget; }

private:
	Ui::AutoDxfCppClass ui;
	MyQOpenGLWidget* m_oglWidget;
	bool m_showMenu;

private slots:
	void OnLoadDxf();
	void OnSplit();
	void OnMouseMoved(const QPointF& pos);
	void OnUpdateTreeModel(QStandardItemModel* model);
	void onTreeItemClicked(const QModelIndex& index);
	void onEntitySelectedInViewport(Entity* entity);
};