#ifndef GRAPHSCENE_H
#define GRAPHSCENE_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsItem>
#include <QDebug>
#include <QMenu>
#include "graphic/graphline.h"
#include "graphic/graphitem.h"


class GraphScene : public QGraphicsScene
{
    Q_OBJECT
    QPointF clickPos;
    bool drawArrow;
    QVector<GraphLine*> vecLine;
    QVector<GraphItem*> vecItem;
    QPointF lastPos;
    GraphItem* removedItem;
    GraphLine* line;
    GraphPoint* current_point;

    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent) override;

public:
    explicit GraphScene(QObject *parent = nullptr);
    void deleteItem(QGraphicsItem* item);
    void addTypeItem(GraphItem::TypeItem type);
    QVector<GraphItem*> getGraphItems();
    bool focusGraphItem();
    GraphItem* lastRemoveItem();
    void createRelationsgGraphItems(GraphItem* source_item, GraphPoint* source_point, GraphItem* dest_item);

public slots:
    void triggerPoint(GraphPoint* point);

signals:
    void addGraphItem();
    void removeGraphItem();
    void clickMouse();
};

#endif // GRAPHSCENE_H
