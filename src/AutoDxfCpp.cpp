#include "AutoDxfCpp.h"
#include "myqopenglwidget.h"
#include <QVBoxLayout>

AutoDxfCpp::AutoDxfCpp(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    // Create your OpenGL widget
    MyQOpenGLWidget* oglWidget = new MyQOpenGLWidget(this);

    // Optionally, add it to a layout
    ui.centralWidget->setLayout(new QVBoxLayout());
    ui.centralWidget->layout()->addWidget(oglWidget);
}

AutoDxfCpp::~AutoDxfCpp()
{
}