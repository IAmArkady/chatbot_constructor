#include "graphview.h"

constexpr int GRID_SIZE = 10;
constexpr qreal SCALE_FACTOR = 0.06;
constexpr qreal MIN_SCALE = 0.15;
constexpr qreal MAX_SCALE = 3.5;

GraphView::GraphView(QWidget* parent) : QGraphicsView(parent) {
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setRenderHint(QPainter::Antialiasing);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setSceneRect(0, 0, 5000, 5000);
    centerOn(2500, 2500);
    viewport()->update();
}

void GraphView::drawBackground(QPainter *painter, const QRectF &rect) {
    painter->setPen(QPen(Qt::lightGray, 1));

    for (qreal x = std::floor(rect.left() / GRID_SIZE) * GRID_SIZE; x < rect.right(); x += GRID_SIZE)
        painter->drawLine(QLineF(x, rect.top(), x, rect.bottom()));

    for (qreal y = std::floor(rect.top() / GRID_SIZE) * GRID_SIZE; y < rect.bottom(); y += GRID_SIZE)
        painter->drawLine(QLineF(rect.left(), y, rect.right(), y));
}

bool GraphView::viewportHoverWidget() {
    if (QWidget* sideWidget = viewport()->findChild<QWidget*>("sideWidget"))
        return sideWidget->underMouse();
    return false;
}

void GraphView::mousePressEvent(QMouseEvent* event) {
    if (viewportHoverWidget()) return;

    if (event->button() == Qt::MiddleButton || event->button() == Qt::RightButton) {
        clickPos = event->pos();
        if (event->button() == Qt::MiddleButton)
            viewport()->setCursor(Qt::OpenHandCursor);
    }
    QGraphicsView::mousePressEvent(event);
}

void GraphView::mouseReleaseEvent(QMouseEvent *event) {
    if (event->button() == Qt::MiddleButton)
        viewport()->setCursor(Qt::ArrowCursor);
    QGraphicsView::mouseReleaseEvent(event);
}

void GraphView::mouseDoubleClickEvent(QMouseEvent *event) {
    if (!viewportHoverWidget())
        QGraphicsView::mouseDoubleClickEvent(event);
}

void GraphView::mouseMoveEvent(QMouseEvent* event) {
    if (viewportHoverWidget()) return;

    if (event->buttons() & Qt::MiddleButton) {
        QPointF offset = (clickPos - event->pos()) * 0.8;
        translate(-offset.x(), -offset.y());
        clickPos = event->pos();
        viewport()->setCursor(Qt::ClosedHandCursor);
    }
    QGraphicsView::mouseMoveEvent(event);
}

void GraphView::wheelEvent(QWheelEvent *event){
    if(viewportHoverWidget())
        return;

    qreal determinant = transform().determinant();
    if ((determinant < 0.15 && event->angleDelta().y() < 0) ||
        (determinant > 3.5 && event->angleDelta().y() > 0))
        return;

    qreal delta = event->angleDelta().y() < 0 ? 1-SCALE_FACTOR : 1+SCALE_FACTOR;
    QPointF targetViewportPos = event->pos(),
        targetScenePos = mapToScene(event->pos());
    scale(delta, delta);

    QPointF deltaViewportPos = targetViewportPos - QPointF(viewport()->width() / 2.0, viewport()->height() / 2.0),
        viewportCenter = mapFromScene(targetScenePos) - deltaViewportPos;
    centerOn(mapToScene(viewportCenter.toPoint()));

    scene()->update();
    qobject_cast<QWidget*>(viewport())->resize(0, 0);
}

void GraphView::triggerMenu(QAction* action) {
    auto* graphScene = qobject_cast<GraphScene*>(scene());
    if (!graphScene) return;

    switch (action->data().toInt()) {
    case 1: graphScene->addTypeItem(GraphItem::Keywords); break;
    case 2: graphScene->addTypeItem(GraphItem::Messages); break;
    case 3: graphScene->addTypeItem(GraphItem::Inputs); break;
    case 4: graphScene->addTypeItem(GraphItem::Actions); break;
    case 5: graphScene->addTypeItem(GraphItem::Requests); break;
    default: break;
    }
}

void GraphView::triggerDelMenu(QAction* action) {
    if (action->data().toInt() == 1) {
        auto* graphScene = qobject_cast<GraphScene*>(scene());
        if (graphScene)
            graphScene->deleteItem(graphScene->focusItem());
    }
}

void GraphView::contextMenuEvent(QContextMenuEvent *event) {
    if (viewportHoverWidget()) return;

    QMenu menu(this);
    menu.setCursor(Qt::PointingHandCursor);
    menu.setStyleSheet("QMenu{border:2px solid rgba(150,150,150,150); font-size:16px;} QMenu::item{}");

    auto* graphScene = qobject_cast<GraphScene*>(scene());
    if (graphScene && graphScene->focusGraphItem()) {
        QAction* deleteAction = menu.addAction(QIcon(":/images/delete.png"), "Удалить");
        deleteAction->setData(1);
        connect(&menu, &QMenu::triggered, this, &GraphView::triggerDelMenu);
    }
    else {
        struct { const char* name; int id; } items[] = {
            {"Ключевые слова", 1},
            {"Сообщения", 2},
            {"Ввод", 3},
            {"Действия", 4},
            {"Запросы", 5}
        };
        for (const auto& item : items)
            menu.addAction(item.name)->setData(item.id);

        connect(&menu, &QMenu::triggered, this, &GraphView::triggerMenu);
    }
    menu.exec(event->globalPos());
}
