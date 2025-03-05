#include "graphline.h"

GraphLine::GraphLine(QGraphicsItem *parent): QGraphicsItem(parent)
{
    startPos = QPointF(0, 0);
    endPos = QPointF(0, 0);
    startItem = nullptr;
    endItem = nullptr;

    line.setPoints(startPos, endPos);

    for(int i = 0; i < 5; i++){
        vecArow.append(new GraphArow(this));
        connect(vecArow[i], &GraphArow::changeFocus, this, &GraphLine::changeFocus);
    }
}

QRectF GraphLine::boundingRect() const{
    return QRectF(0, 0, 5000, 5000).normalized();
}

void GraphLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    line.setPoints(startItem ? QPointF(startItem->scenePos().x()+startItem->boundingRect().width(), startItem->scenePos().y()+startItem->boundingRect().height()/2*startItem->scale()):startPos,
                 endItem ? mapToScene(QPointF(endItem->scenePos().x(), endItem->scenePos().y()+endItem->boundingRect().height()/2)) : endPos);

    if(line.x2() - line.x1()  <= 80){
        vecArow.first()->setStartPos(line.p1());
        vecArow.first()->setEndPos(QPointF(line.x1() + 40, line.y1()));

        vecArow[1]->setStartPos(vecArow.first()->getEndPos());
        vecArow[1]->setEndPos(QPointF(vecArow.first()->getEndPos().x(), line.center().y()));

        vecArow[2]->setStartPos(vecArow[1]->getEndPos());
        vecArow[2]->setEndPos(QPointF(line.x2() - 40, vecArow[1]->getEndPos().y()));

        vecArow[3]->setStartPos(vecArow[2]->getEndPos());
        vecArow[3]->setEndPos(QPointF(vecArow[2]->getEndPos().x(),line.y2()));

        vecArow.last()->setStartPos(vecArow[3]->getEndPos());
        vecArow.last()->setEndPos(line.p2());
    }
    else{
        vecArow.first()->setStartPos(line.p1());
        vecArow.first()->setEndPos(QPointF(line.center().x(), line.y1()));

        vecArow[1]->setStartPos(QPointF(line.center().x(), line.y1()));
        vecArow[1]->setEndPos(QPointF(line.center().x(), line.y2()));

        vecArow[2]->setStartPos(QPointF(0,0));
        vecArow[2]->setEndPos(QPointF(0,0));

        vecArow[3]->setStartPos(QPointF(0,0));
        vecArow[3]->setEndPos(QPointF(0,0));

        vecArow.last()->setStartPos(QPointF(line.center().x(), line.y2()));
        vecArow.last()->setEndPos(line.p2());
    }
    vecArow.last()->setDrawArrow(true);
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

void GraphLine::changeFocus(){
    GraphArow* arow = qobject_cast<GraphArow*>(sender());
    if(arow->hasFocus()){
        for(auto arow: vecArow)
            arow->setColor(QColor(0, 255, 0, 150));
        setZValue(1);
    }
    else{
        for(auto arow: vecArow)
            arow->setColor(Qt::black);
        setZValue(0);
    }
}

QGraphicsItem* GraphLine::getStartItem(){
    return startItem;
}

QGraphicsItem* GraphLine::getEndItem(){
    return endItem;
}

void GraphLine::setStartItem(QGraphicsItem* item){
    startItem = item;
}
void GraphLine::setEndItem(QGraphicsItem* item){
    endItem = item;
}
void GraphLine::setStartPos(QPointF pos){
    startPos = pos;
}
void GraphLine::setEndPos(QPointF pos){
    endPos = pos;
}
