#include "graphscene.h"

GraphScene::GraphScene(QObject *parent) : QGraphicsScene(parent)
{
    this->setSceneRect(QRectF(0, 0, 5000, 5000));
    drawArrow = false;
    removedItem = nullptr;
    current_point = nullptr;
}


void GraphScene::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent){
    emit clickMouse();
    clickPos = mouseEvent->scenePos();
    if (mouseEvent->button() == Qt::LeftButton){
        if(drawArrow){
            GraphItem* start_item = qgraphicsitem_cast<GraphItem*>(vecLine.last()->getStartItem()->parentItem());
            if(focusItem()){
                GraphItem* item  = qgraphicsitem_cast<GraphItem*>(focusItem());
                if(vecItem.indexOf(item) > -1 && vecLine.last()->getStartItem()->parentItem() != qgraphicsitem_cast<QGraphicsItem*>(item)){
                   if(start_item->getPoints().indexOf(current_point) == 0)
                       start_item->setNextItem(focusItem());
                   else
                       start_item->setNextItemButton(current_point, focusItem());
                   QColor title_color = start_item->getTitleColor();
                   title_color.setAlpha(255);
                   current_point->setFillColor(title_color);
                   vecLine.last()->setEndItem(focusItem());
                }
                else{
                    start_item->setNextItem(nullptr);
                    start_item->setNextItemButton(current_point, nullptr);
                    delete vecLine.last();
                    vecLine.removeLast();
                }
            }
            else{
                start_item->setNextItem(nullptr);
                start_item->setNextItemButton(current_point, nullptr);
                delete vecLine.last();
                vecLine.removeLast();
            }

            drawArrow = false;
            for(auto temp: vecItem)
                if(temp->getType() == GraphItem::Keywords){
                    temp->setEnabled(true);
                    temp->setOpacity(1);
                }
            update();
        }
    }

    if (mouseEvent->button() == Qt::MiddleButton)
        return;
    QGraphicsScene::mousePressEvent(mouseEvent);
}

void GraphScene::mouseMoveEvent(QGraphicsSceneMouseEvent *mouseEvent){
    if(drawArrow){
        vecLine.last()->clearFocus();
        vecLine.last()->setEndPos(mouseEvent->scenePos());
        update();
    }
    QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void GraphScene::addTypeItem(GraphItem::TypeItem type){
    GraphItem* item = new GraphItem(type);
    connect(item, &GraphItem::triggerPoint, this, &GraphScene::triggerPoint);
    connect(item, &GraphItem::deletePoint, this, &GraphScene::deleteItem);
    item->setPos(clickPos);
    addItem(item);
    vecItem.append(item);
    emit addGraphItem();
}

QVector<GraphItem*> GraphScene::getGraphItems(){
    return vecItem;
}

void GraphScene::triggerPoint(GraphPoint* point){
    GraphItem* item = qobject_cast<GraphItem*>(sender());
    GraphLine* line = new GraphLine();
    current_point = point;

    for(auto temp: vecLine)
        if (temp->getStartItem() == qgraphicsitem_cast<GraphItem*>(point)){
            vecLine.removeOne(temp);
            delete temp;
        }

    line->setStartItem(qgraphicsitem_cast<QGraphicsItem*>(point));
    line->setEndPos(clickPos);
    vecLine.append(line);
    addItem(line);
    point->setFillColor(QColor(170,170,170));

    drawArrow = true;
    for(auto temp: vecItem)
        if(temp->getType() == GraphItem::Keywords && temp != item){
            temp->setEnabled(false);
            temp->setOpacity(0.3);
        }
}


bool GraphScene::focusGraphItem(){
    if(!focusItem())
        return false;
    if(vecLine.indexOf(qgraphicsitem_cast<GraphLine*>(focusItem()->parentItem())) > -1 ||
            vecItem.indexOf(qgraphicsitem_cast<GraphItem*>(focusItem())) > -1)
        return true;
    return false;
}

void GraphScene::deleteItem(QGraphicsItem *item){
    GraphLine* graphLine;
    GraphItem* graphItem;
    GraphPoint* graphPoint;

    graphPoint=qgraphicsitem_cast<GraphPoint*>(item);
    for(auto line: vecLine){
        if (line->getStartItem() == graphPoint){
            vecLine.removeOne(line);
            removeItem(line);
            delete line;
        }
    }
    update();


    if(vecLine.indexOf(graphLine=qgraphicsitem_cast<GraphLine*>(item->parentItem())) > -1){
        for(auto point: qgraphicsitem_cast<GraphItem*>(graphLine->getStartItem()->parentItem())->getPoints())
            if(qgraphicsitem_cast<QGraphicsItem*>(point) == graphLine->getStartItem()){
                GraphItem* start_item = qgraphicsitem_cast<GraphItem*>(graphLine->getStartItem()->parentItem());
                if (start_item->getPoints().indexOf(point) == 0)
                    start_item->setNextItem(nullptr);
                else
                    start_item->setNextItemButton(point, nullptr);
                point->setFillColor(QColor(170,170,170));
            }
        vecLine.removeOne(graphLine);
        removeItem(item);
        delete graphLine;
    }

    if(vecItem.indexOf(graphItem=qgraphicsitem_cast<GraphItem*>(item)) > -1){
        QVector<GraphLine*> vecTemp;
        for(auto line: vecLine)
            if(line->getStartItem()->parentItem() == qgraphicsitem_cast<QGraphicsItem*>(graphItem) || line->getEndItem() == qgraphicsitem_cast<QGraphicsItem*>(graphItem))
                vecTemp.append(line);

        for(auto temp: vecTemp){
            GraphItem* start_item = qgraphicsitem_cast<GraphItem*>(temp->getStartItem()->parentItem());
            GraphPoint* point = qgraphicsitem_cast<GraphPoint*>(temp->getStartItem());
            if (start_item->getPoints().indexOf(point) == 0)
                start_item->setNextItem(nullptr);
            else
                start_item->setNextItemButton(point, nullptr);
            point->setFillColor(QColor(170,170,170));
            vecLine.removeOne(temp);
            removeItem(temp);
            delete temp;
        }

        vecItem.removeOne(graphItem);
        removeItem(graphItem);
        delete graphItem;
        removedItem = graphItem;
        emit removeGraphItem();
    }
}

void GraphScene::createRelationsgGraphItems(GraphItem *source_item, GraphPoint *source_point, GraphItem *dest_item){
    GraphLine* line = new GraphLine();
    line->setStartItem(source_point);
    line->setEndItem(dest_item);
    vecLine.append(line);
    addItem(line);

    if(source_item->getPoints().indexOf(source_point) == 0)
        source_item->setNextItem(dest_item);
    else
        source_item->setNextItemButton(source_point, dest_item);

    QColor title_color = source_item->getTitleColor();
    title_color.setAlpha(255);
    source_point->setFillColor(title_color);
    update();
}

GraphItem* GraphScene::lastRemoveItem(){
    return removedItem;
}
