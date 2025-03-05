#include "grapharrow.h"

GraphArow::GraphArow(QGraphicsItem *parent): QGraphicsLineItem(parent)
{
    setFlag(QGraphicsLineItem::ItemIsSelectable, true);
    setFlag(QGraphicsLineItem::ItemIsFocusable, true);
    setAcceptHoverEvents(true);
    startPos = QPointF(0, 0);
    endPos = QPointF(0, 0);

    drawArrow = false;

    pen.setColor(Qt::black);
    pen.setWidth(3);
    pen.setStyle(Qt::SolidLine);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    setPen(pen);
}

QRectF GraphArow::boundingRect() const{
//    qreal extra = (pen().width() + 20) / 2.0;
//    return QRectF(line().p1(), QSizeF(line().p2().x() - line().p1().x(),line().p2().y() - line().p1().y()))
//            .normalized()
//            .adjusted(-extra, -extra, extra, extra);
    return QRectF(0, 0, 5000, 5000).normalized();
}

void GraphArow::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    painter->setPen(pen);
    painter->setBrush(Qt::green);
    QBrush brush(QColor(250,0,0,255));
    setLine(QLineF(startPos, endPos));
    painter->drawLine(line());

    if(drawArrow){
        QPolygonF tr;
        tr.append(line().p2());
        tr.append(QPointF(line().x2()-15,line().y2()+5));
        tr.append(QPointF(line().x2()-15,line().y2()-5));
        painter->drawPolygon(tr);
    }
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

void GraphArow::setDrawArrow(bool flag){
    drawArrow = flag;
}

void GraphArow::setColor(QColor color){
    pen.setColor(color);
}

void GraphArow::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(event->modifiers() & Qt::ControlModifier)
        event->ignore();
    QGraphicsItem::mousePressEvent(event);
}

void GraphArow::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    setCursor(Qt::CursorShape::SizeAllCursor);
    setFocus();
    emit changeFocus();
    QGraphicsItem::hoverEnterEvent(event);
}

void GraphArow::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    setCursor(Qt::CustomCursor);
    clearFocus();
    emit changeFocus();
    QGraphicsItem::hoverLeaveEvent(event);
}


void GraphArow::setStartPos(const QPointF& pos){
    startPos = pos;
}

void GraphArow::setEndPos(const QPointF& pos){
    endPos = pos;
}

QPointF GraphArow::getStartPos(){
    return startPos;
}

QPointF GraphArow::getEndPos(){
    return endPos;
}
