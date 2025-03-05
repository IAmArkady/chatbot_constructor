#ifndef GRAPHARROW_H
#define GRAPHARROW_H
#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QApplication>
#include <QObject>
#include "graphic/graphitem.h"


class GraphArow : public QObject, public QGraphicsLineItem
{
    Q_OBJECT
    QPointF startPos, endPos;
    QPen pen;
    bool drawArrow;
public:
    GraphArow(QGraphicsItem *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QRectF boundingRect() const override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    void setStartPos(const QPointF& pos);
    void setEndPos(const QPointF& pos);
    QPointF getStartPos();
    QPointF getEndPos();
    void setColor(QColor color);
    void setDrawArrow(bool flag);

signals:
    void changeFocus();
};

#endif // GRAPHARROW_H
