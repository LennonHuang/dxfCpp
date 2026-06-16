#include "AutoDxfCpp.h"
#include "myqopenglwidget.h"
#include "AutoDxfHelper.h"
#include "Entities/Polyline.h"
#include <QVBoxLayout>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <algorithm>

AutoDxfCpp::AutoDxfCpp(bool showMenu, QWidget* parent)
    : QMainWindow(parent), m_showMenu(showMenu)
{
    ui.setupUi(this);

    // Hide menu bar for split windows
    if (!m_showMenu) {
        ui.menuBar->hide();
    }

    // Create your OpenGL widget
    m_oglWidget = new MyQOpenGLWidget(this);

    // Optionally, add it to a layout
    ui.centralWidget->setLayout(new QVBoxLayout());
    ui.centralWidget->layout()->addWidget(m_oglWidget);

    // Top Menu
	connect(ui.actionLoad, &QAction::triggered, this, &AutoDxfCpp::OnLoadDxf);
	connect(ui.actionClear, &QAction::triggered, m_oglWidget, &MyQOpenGLWidget::OnClearDxf);
	connect(ui.actionSplit, &QAction::triggered, this, &AutoDxfCpp::OnSplit);
    connect(m_oglWidget, &MyQOpenGLWidget::MouseMoved, this, &AutoDxfCpp::OnMouseMoved);

    // Tree View
    ui.treeView->setHeaderHidden(true);
    connect(m_oglWidget, &MyQOpenGLWidget::UpdateTreeModel, this, &AutoDxfCpp::OnUpdateTreeModel);
    connect(ui.treeView, &QTreeView::clicked,
        this, &AutoDxfCpp::onTreeItemClicked);
	connect(m_oglWidget, &MyQOpenGLWidget::EntitySelected,
		this, &AutoDxfCpp::onEntitySelectedInViewport);
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

void AutoDxfCpp::OnSplit()
{
    bool ok;
    QString cutterLayer = QInputDialog::getText(
        this,
        tr("Split"),
        tr("Cutter layer name:"),
        QLineEdit::Normal,
        "1",
        &ok);

    if (!ok || cutterLayer.isEmpty())
        return;

    const auto& entities = m_oglWidget->getEntities();

    std::vector<Polyline*> trimlines;
    std::vector<Polyline*> ogPolylines;

    for (const auto& entity : entities) {
        Polyline* poly = dynamic_cast<Polyline*>(entity.get());
        if (!poly) continue;

        QString layer = QString::fromUtf8(entity->getLayer());
        if (layer == cutterLayer) {
            trimlines.push_back(poly);
        } else {
            ogPolylines.push_back(poly);
        }
    }

    // For each ogPly, collect all intersections from all trimlines, sort, and split
    std::vector<std::shared_ptr<Entity>> trimmedEntities;
    std::vector<glm::vec2> allIntersectionPts;

    for (auto* ogPly : ogPolylines) {
        // Accumulate intersections from all trimlines for this ogPly
        std::vector<AutoDxfHelper::IntersectionPoint> ips;
        for (auto* trimline : trimlines) {
            auto hits = AutoDxfHelper::PolylineIntersections(
                ogPly->getPolyVertices(), ogPly->getIsClosed(),
                trimline->getPolyVertices(), trimline->getIsClosed());
            ips.insert(ips.end(), hits.begin(), hits.end());
        }

        // Collect intersection points for display
        for (const auto& ip : ips) {
            allIntersectionPts.push_back(ip.point);
        }

        // Sort by segmentIndex, then by parameter
        std::sort(ips.begin(), ips.end(),
            [](const AutoDxfHelper::IntersectionPoint& a,
               const AutoDxfHelper::IntersectionPoint& b) {
                if (a.segmentIndex != b.segmentIndex)
                    return a.segmentIndex < b.segmentIndex;
                return a.parameter < b.parameter;
            });

        // Split the ogPly at intersection points
        auto subPolys = AutoDxfHelper::SplitPolyline(
            ogPly->getPolyVertices(), ogPly->getIsClosed(), ips);

        // Create Polyline entities from trimmed results
        for (auto& verts : subPolys) {
            if (verts.size() < 2) continue;
            auto trimmed = std::make_shared<Polyline>(verts, false);
            trimmed->setColor(0.0f, 1.0f, 0.0f); // Green for trimmed
            trimmed->setLayer(ogPly->getLayer());
            trimmedEntities.push_back(trimmed);
        }
    }

    qDebug() << "Trimlines:" << trimlines.size()
             << "OgPolylines:" << ogPolylines.size()
             << "Intersection points:" << allIntersectionPts.size()
             << "Trimmed sub-polylines:" << trimmedEntities.size();

    // Create split window and display results
    AutoDxfCpp* splitWindow = new AutoDxfCpp(false);
    splitWindow->setAttribute(Qt::WA_DeleteOnClose);
    splitWindow->setWindowTitle("AutoDxfCpp - Split");
    splitWindow->show();

    MyQOpenGLWidget* targetWidget = splitWindow->getOglWidget();
    QTimer::singleShot(0, targetWidget,
        [targetWidget, trimmedEntities, allIntersectionPts]() {
            targetWidget->addEntities(trimmedEntities);
            targetWidget->addIntersectionPoints(allIntersectionPts);
        });
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

    auto entity = item->data(Qt::UserRole).value<std::shared_ptr<Entity>>();
    if (entity) {
        qDebug() << "Clicked entity type:" << QString::fromStdString(entity->getType());
        // Do something with the actual entity
        m_oglWidget->highlightSelectedEntity(entity.get());
    }
    else 
    {
        m_oglWidget->highlightSelectedEntity(nullptr);
    }
}

void AutoDxfCpp::onEntitySelectedInViewport(Entity* entity)
{
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui.treeView->model());
    if (!model) return;

    if (!entity) {
        ui.treeView->clearSelection();
        return;
    }

    // Search through tree items to find the matching entity pointer
    QStandardItem* root = model->item(0); // "Dxf Entities" item
    if (!root) return;

    for (int i = 0; i < root->rowCount(); ++i) {
        QStandardItem* item = root->child(i);
        auto itemEntity = item->data(Qt::UserRole).value<std::shared_ptr<Entity>>();
        if (itemEntity.get() == entity) {
            ui.treeView->setCurrentIndex(item->index());
            return;
        }
    }
    ui.treeView->clearSelection();
}
