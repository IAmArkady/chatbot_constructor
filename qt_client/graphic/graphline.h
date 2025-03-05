#ifndef GRAPHLINE_H
#define GRAPHLINE_H
#include <QGraphicsItem>
#include <QObject>
#include "graphic/grapharrow.h"
#include "graphic/graphitem.h"



class GraphLine :public QObject, public QGraphicsItem
{
    Q_OBJECT
    QPointF startPos, endPos;
    QGraphicsItem* startItem, *endItem;
    QLineF line;
    QVector<GraphArow*> vecArow;
public:
    GraphLine(QGraphicsItem *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
    QRectF boundingRect() const;
    QGraphicsItem* getStartItem();
    QGraphicsItem* getEndItem();
    void setStartItem(QGraphicsItem* item);
    void setEndItem(QGraphicsItem* item);
    void setStartPos(QPointF pos);
    void setEndPos(QPointF pos);
public slots:
    void changeFocus();
};

#endif // GRAPHLINE_H
