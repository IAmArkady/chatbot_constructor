#include "graphpoint.h"

GraphPoint::GraphPoint(QGraphicsItem *parent):QGraphicsItem(parent)
{
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setAcceptHoverEvents(true);
    setZValue(1);
    fill_color = QColor(170,170,170);
}

QRectF GraphPoint::boundingRect() const{
    return QRectF(0, 0, 20, 20);
}

void GraphPoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QBrush brush(fill_color);
    QRect ellipse = QRect(0, 0, boundingRect().width(), boundingRect().height());
    painter->setBrush(brush);
    if (isSelected()){
        painter->setBackground(brush);
        painter->setBrush(brush);
        painter->setPen(QColor(0,255,0,255));
    }
    if(hasFocus()){

    }

    painter->drawEllipse(ellipse);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

void GraphPoint::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(event->button()==Qt::LeftButton)
        emit triggered();
    QGraphicsItem::mousePressEvent(event);
}

void GraphPoint::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    float scale = 1.3;
    setCursor(Qt::CursorShape::PointingHandCursor);
    setFocus();
    fixPos = pos();
    setScale(scale);
    setPos(pos().x() - (boundingRect().width()*scale-boundingRect().width())/2, pos().y() - (boundingRect().height()*scale-boundingRect().height())/2);
    QGraphicsItem::hoverEnterEvent(event);
}

void GraphPoint::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    setCursor(Qt::CustomCursor);
    clearFocus();
    setScale(1);
    setPos(fixPos);
    QGraphicsItem::hoverLeaveEvent(event);
}

void GraphPoint::setFillColor(QColor color){
    fill_color = color;
}

QColor GraphPoint::getFillColor(){
    return fill_color;
}
