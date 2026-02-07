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
    connect(m_oglWidget, &MyQOpenGLWidget::MouseMoved, this, &AutoDxfCpp::OnMouseMoved);

    // Tree View
    ui.treeView->setHeaderHidden(true);
    connect(m_oglWidget, &MyQOpenGLWidget::UpdateTreeModel, this, &AutoDxfCpp::OnUpdateTreeModel);
    connect(ui.treeView, &QTreeView::clicked,
        this, &AutoDxfCpp::onTreeItemClicked);
}

AutoDxfCpp::~AutoDxfCpp()
{
}

void AutoDxfCpp::OnMouseMoved(const QPointF& pos)
{
    ui.statusBar->showMessage(
        QString("X: %1 mm, Y: %2 mm").arg(pos.x(), 0, 'f', 2)
        .arg(pos.y(), 0, 'f', 2));
}

void AutoDxfCpp::OnLoadDxf()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open DXF File"), "", tr("DXF Files (*.dxf)"));
    if (fileName.isEmpty())
        return;

    m_oglWidget->loadDxf(fileName);
}

void AutoDxfCpp::OnUpdateTreeModel(QStandardItemModel* model)
{
    model->setParent(this);// Set MainWindow as parent to take ownership

    // Replace existing model if needed
    if (ui.treeView->model()) {
        ui.treeView->model()->deleteLater();
    }

    ui.treeView->setModel(model);
}

void AutoDxfCpp::onTreeItemClicked(const QModelIndex& index) {
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.treeView->model());
    if (!model) return;

    QStandardItem* item = model->itemFromIndex(index);
    if (!item) return;

    Entity* entity = reinterpret_cast<Entity*>(item->data(Qt::UserRole).value<void*>());
    if (entity) {
        qDebug() << "Clicked entity type:" << QString::fromStdString(entity->getType());
        // Do something with the actual entity
        m_oglWidget->highlightSelectedEntity(entity);
    }
    else 
    {
        m_oglWidget->highlightSelectedEntity(nullptr);
    }
}