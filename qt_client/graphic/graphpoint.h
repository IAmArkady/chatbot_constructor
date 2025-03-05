#ifndef GRAPHPOINT_H
#define GRAPHPOINT_H
#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QApplication>

class GraphPoint :public QObject, public QGraphicsItem
{
    Q_OBJECT
    QPointF fixPos;
    QColor fill_color;
public:
    explicit GraphPoint(QGraphicsItem *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
    QRectF boundingRect() const;
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    void setFillColor(QColor color);
    QColor getFillColor();

signals:
     void triggered();
};

#endif // GRAPHPOINT_H
