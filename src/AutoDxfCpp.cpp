#include "AutoDxfCpp.h"
#include "myqopenglwidget.h"
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMessageBox>

AutoDxfCpp::AutoDxfCpp(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // Create your OpenGL widget
    m_oglWidget = new MyQOpenGLWidget(this);

    // Optionally, add it to a layout
    ui.centralWidget->setLayout(new QVBoxLayout());
    ui.centralWidget->layout()->addWidget(m_oglWidget);

    // Top Menu
	connect(ui.actionLoad, &QAction::triggered, this, &AutoDxfCpp::OnLoadDxf);
	connect(ui.actionClear, &QAction::triggered, m_oglWidget, &MyQOpenGLWidget::OnClearDxf);
}

AutoDxfCpp::~AutoDxfCpp()
{
}

void AutoDxfCpp::OnLoadDxf()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open DXF File"), "", tr("DXF Files (*.dxf)"));
    if (fileName.isEmpty())
        return;

    m_oglWidget->loadDxf(fileName);
}