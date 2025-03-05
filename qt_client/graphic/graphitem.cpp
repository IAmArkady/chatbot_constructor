#include "graphitem.h"

GraphItem::GraphItem(TypeItem typeItem, QGraphicsItem *parent): QGraphicsItem(parent)
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsFocusable, true);
    setAcceptHoverEvents(true);
    setZValue(2);
    width_rect = 3;
    type = typeItem;

    boundingRect_width = 225;
    boundingRect_height = 100;

    next_item = nullptr;

    block_style = Qt::SolidLine;

    GraphPoint* point = new GraphPoint(this);
    point->setPos(boundingRect().width()-width_rect-point->boundingRect().width(), (boundingRect().height()-point->boundingRect().height())/2+7);
    connect(point, &GraphPoint::triggered, this, &GraphItem::slotTriggerPoint);
    vec_points.append(point);
}

QRectF GraphItem::boundingRect() const{
    return QRectF(0, 0, boundingRect_width, boundingRect_height);
}


void GraphItem::addTextBlock(QString text){
    vec_text_blocks.append({text});
    QFont font("Verdana", 8);
    QFontMetrics fontMetrics(font);
    qreal y = boundingRect().top()+50;
    qreal height = 0;
    QRectF text_rect = fontMetrics.boundingRect(boundingRect().left()+15, y, boundingRect().width()-45, 0, Qt::TextWrapAnywhere, text);
    height += text_rect.height()+5;
    boundingRect_height += height;
}

void GraphItem::removeTextBlock(int index){
    if(index > -1 && index < vec_text_blocks.size()){
        QFont font("Verdana", 8);
        QFontMetrics fontMetrics(font);
        qreal y = boundingRect().top()+50;
        QRectF text_rect = fontMetrics.boundingRect(boundingRect().left()+15, y, boundingRect().width()-45, 0, Qt::TextWrapAnywhere, vec_text_blocks[index].text);
        boundingRect_height -= (text_rect.height()+5);
        vec_text_blocks.remove(index);
        scene()->update();
    }
}

void GraphItem::removeAllTextBlock(){
    QFont font("Verdana", 8);
    QFontMetrics fontMetrics(font);
    qreal y = boundingRect().top()+50;
    for(int i = 0; i < vec_text_blocks.size(); i++){
        QRectF text_rect = fontMetrics.boundingRect(boundingRect().left()+15, y, boundingRect().width()-45, 0, Qt::TextWrapAnywhere, vec_text_blocks[i].text);
        boundingRect_height -= (text_rect.height()+5);
    }
    vec_text_blocks.clear();
    scene()->update();
}

void GraphItem::updateTextBlock(int index, QString text){
    if(index > -1 && index < vec_text_blocks.size()){
        QFont font("Verdana", 8);
        QFontMetrics fontMetrics(font);
        qreal y = boundingRect().top()+50;
        QRectF text_rect = fontMetrics.boundingRect(boundingRect().left()+15, y, boundingRect().width()-45, 0, Qt::TextWrapAnywhere, vec_text_blocks[index].text),
                new_text_rect = fontMetrics.boundingRect(boundingRect().left()+15, y, boundingRect().width()-45, 0, Qt::TextWrapAnywhere, text);
        boundingRect_height =  boundingRect_height - text_rect.height() + new_text_rect.height();
        vec_text_blocks[index].text = text;
        scene()->update();
    }
}

QVector<GraphItem::text_block> GraphItem::getTextBlocks(){
    return vec_text_blocks;
}


void GraphItem::addBlockButtons(){
    QVector<button_graph> vec;
    vec_blocks.push_back(vec);
    boundingRect_height += 15;
}

void GraphItem::addButton(int index, QString title, QColor color){
    boundingRect_height += 40;
    if(index > -1 && index < vec_blocks.size()){
        GraphPoint* point = new GraphPoint(this);
        connect(point, &GraphPoint::triggered, this, &GraphItem::slotTriggerPoint);
        vec_points.append(point);
        vec_blocks[index].push_back({title, point, nullptr, color});
    }
}

void GraphItem::updateButton(int index_block, int index_button, QString title, QColor color){
    if(index_block > -1 && index_block < vec_blocks.size()){
        if(index_button > -1 && index_button < vec_blocks[index_block].size()){
            vec_blocks[index_block][index_button].title = title;
            vec_blocks[index_block][index_button].color = color;
        }
    }
}

void GraphItem::removeBlockButtons(int index){
    if(index > -1 && index < vec_blocks.size()){
        boundingRect_height -= 15;
         boundingRect_height =  boundingRect_height - vec_blocks.at(index).size() * 40;
         for(auto block_button: vec_blocks[index]){
             GraphPoint* point = block_button.point;
             vec_points.removeOne(point);
             delete point;
             emit deletePoint(point);
         }
         vec_blocks.removeAt(index);
    }
}

void GraphItem::removeButton(int index_block, int index_button){
    if(index_block > -1 && index_block < vec_blocks.size()){
        if(index_button > -1 && index_button < vec_blocks[index_block].size()){
            GraphPoint* point = vec_blocks.at(index_block).at(index_button).point;
            vec_points.removeOne(point);
            point->deleteLater();
            vec_blocks[index_block].removeAt(index_button);
            boundingRect_height -= 40;
            emit deletePoint(point);
        }
    }
}

void GraphItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
    QBrush brush(QColor(150,150,150,200));
    QRect rect = QRect(boundingRect().x()+width_rect, boundingRect().y()+width_rect,boundingRect().width()-width_rect*2,boundingRect().height()-width_rect*2);
    painter->setRenderHint(QPainter::Antialiasing);
    brush.setStyle(Qt::BrushStyle::SolidPattern);

    //Цвет заливки
    painter->setBrush(QColor(255,255,255,255));

    //Выбран или сфокусирован
    if(hasFocus() && !isSelected())
        brush.setColor(QColor(0, 200, 200, 255));
    if (isSelected())
        brush.setColor(QColor(0,255,81,255));

    //Цвет границ
    painter->setPen(QPen(brush, width_rect));
    qreal offset = width_rect/2.5, corner_radius = 25.0 - width_rect, height = 30;
    QPainterPath path, path1;
    path.moveTo(rect.x() + corner_radius + offset, rect.y() + offset);
    path.arcTo(rect.x() + offset, rect.y() + offset, 2 * corner_radius, 2 * corner_radius, 90.0, 90.0);
    path.lineTo(rect.x() + offset, rect.y() + height);
    path.lineTo(rect.x() + rect.width() - offset, rect.y() + height);
    path.arcTo(rect.x() + (rect.width() - 2 * corner_radius) - offset, rect.y() + offset, 2 * corner_radius, 2 * corner_radius, 0, 90.0);
    path.lineTo(rect.x() + corner_radius + offset, rect.y()+offset);
    painter->drawPath(path);
    painter->fillPath(path, QBrush(color_title));
    path1.moveTo(rect.x() + corner_radius + offset, rect.y() + rect.height());
    path1.arcTo(rect.x() + (rect.width() - 2 * corner_radius) - offset, rect.y() + (rect.height() - 2 * corner_radius), 2 * corner_radius, 2 * corner_radius, 270.0, 90.0);
    path1.lineTo(rect.x() + rect.width() - offset, rect.y() + height);
    path1.lineTo(rect.x() + offset, rect.y() + height);
    path1.arcTo(rect.x() + offset, rect.y() + (rect.height() - 2 * corner_radius), 2 * corner_radius, 2 * corner_radius, 180.0, 90.0);
//    painter->fillPath(path1, QBrush(Qt::red));
    painter->drawPath(path1);


    //Заголовок
    brush.setColor(QColor(0,0,0));
    painter->setPen(QPen(brush, width_rect));
    painter->setFont(QFont("Arial", 10, QFont::Bold));
    painter->drawText(QRect(0, 2, boundingRect().width(), height), Qt::AlignCenter, title);



    //Блок с текстом
    QFont font("Verdana", 8, QFont::Normal);
    painter->setFont(font);
    QFontMetrics fontMetrics(font);
    qreal y = boundingRect().top()+50;
    for(auto text_block: vec_text_blocks){
        QRectF textRect = fontMetrics.boundingRect(boundingRect().left()+15, y, boundingRect().width()-45, 0, Qt::TextWrapAnywhere, text_block.text);
        if (!text_block.text.isEmpty()){
            painter->drawText(textRect, Qt::AlignTop | Qt::TextWrapAnywhere, text_block.text);
            y += textRect.height() + 5;
        }
    }


//    painter->setFont(QFont("Verdana", 9, QFont::Normal));
//    painter->drawText(QPointF(15, 55), block_title);
//    QFont font("Verdana", 7, QFont::Normal);
//    QFontMetrics fontMetrics(font);
//    QRectF temp_rect = boundingRect().adjusted(15, 60, -15, -40);
//    qreal y = temp_rect.top();
//    QRectF textRect = fontMetrics.boundingRect(temp_rect.left(), y, temp_rect.width(), 0, Qt::TextWrapAnywhere, block_text);
//    if (temp_rect.contains(textRect))
//    {
//        painter->drawText(textRect, Qt::AlignTop | Qt::TextWrapAnywhere, block_text);
//    }
//    else
//    {
//        painter->setPen(Qt::white);
//        painter->setBrush(Qt::white);
//        painter->drawRect(temp_rect);
//        painter->setPen(Qt::black);
//        painter->drawText(textRect, Qt::AlignTop | Qt::TextWrapAnywhere, block_text);
//    }

//    prepareGeometryChange();
////    painter->setFont(font);
////    painter->drawText(boundingRect().adjusted(15, 60, -15, -10), Qt::TextWrapAnywhere, block_text);



    //Кнопки
    painter->setFont(QFont("Verdana", 10, QFont::Normal));
    QRect rect_button;
    if(block_style == Qt::NoPen)
        rect_button = QRect(boundingRect().x()+10, boundingRect().height()-45, boundingRect().width()-20, 30);
    else
        rect_button = QRect(boundingRect().x()+15, boundingRect().height()-55, boundingRect().width()-35, 30);

    for(int i = vec_blocks.size()-1; i > -1; i--){
        if(!vec_blocks[i].isEmpty()){
            QRect rect_block = rect_button;
            rect_block.setX(rect_block.x()-5);
            rect_block.setWidth(rect_block.width()+5);

            for(int j = vec_blocks[i].size()-1; j > -1; j--){
                vec_blocks[i][j].point->setPos(rect_button.width()-vec_blocks[i][j].point->boundingRect().width()/2 - 5, (rect_button.y()+rect_button.height()/2)-vec_blocks[i][j].point->boundingRect().height()/2);
                QColor color_border = vec_blocks[i][j].point->getFillColor().darker(110);
                painter->setPen(QPen(QBrush(color_border), 2));
                painter->setBrush(QBrush(vec_blocks[i][j].color));
                painter->drawRoundedRect(rect_button, 10, 20);
                painter->setPen(QPen(brush, 1));
                painter->drawText(rect_button, Qt::AlignCenter, vec_blocks[i][j].title);
                rect_button.translate(0, -40);
            }
            rect_block.setY(rect_button.y()+35);
            rect_block.setHeight(rect_block.height()+5);
            rect_button.translate(0, -15);

            painter->setBrush(QBrush(QColor(0, 0, 0, 0)));
            painter->setPen(QPen(Qt::gray, 1, block_style));
            painter->drawRoundedRect(rect_block, 5, 5);
        }
    }


//    for(auto button: vec_graph_buttons){
//        QColor color_border = button.point->getFillColor().darker(110);
//        painter->setPen(QPen(QBrush(color_border), 2));
//        painter->drawRoundedRect(button.rect_size, 10, 20);

//        painter->setPen(QPen(brush, 1));
//        painter->drawText(button.rect_size, Qt::AlignCenter, button.title);
//    }

    update();


    switch(getType()){
    case GraphItem::Keywords: {
        break;
    }
    }

    Q_UNUSED(option);
    Q_UNUSED(widget);
}

void GraphItem::mousePressEvent(QGraphicsSceneMouseEvent *event){
    if(event->modifiers() & Qt::ControlModifier)
        event->ignore();
    if(event->button() == Qt::RightButton)
        setFocus();
    QGraphicsItem::mousePressEvent(event);
}

void GraphItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    setCursor(Qt::CursorShape::SizeAllCursor);
    setFocus();
    QGraphicsItem::hoverEnterEvent(event);
}

void GraphItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    setCursor(Qt::CustomCursor);
    clearFocus();
    QGraphicsItem::hoverLeaveEvent(event);
}

void GraphItem::slotTriggerPoint(){
    emit triggerPoint(qobject_cast<GraphPoint*>(sender()));
}

void GraphItem::slotDeletePoint(QObject* point){
    emit deletePoint(qobject_cast<GraphPoint*>(point));
}

GraphItem::TypeItem GraphItem::getType(){
    return type;
}

QVector<GraphPoint*> GraphItem::getPoints(){
    return vec_points;
}

void GraphItem::setText(QString text){
    this->text = text;
}

void GraphItem::setTitle(QString text){
    title = text;
    update();
}

void GraphItem::setTitleColor(QColor color){
    color_title = color;
}

QGraphicsItem* GraphItem::getNextItem(){
    return next_item;
}

void GraphItem::setNextItem(QGraphicsItem *item){
    next_item = item;
}

QColor GraphItem::getTitleColor(){
    return color_title;
}

QString GraphItem::getTitle(){
    return title;
}

QVector<GraphItem::button> GraphItem::getButtons(){
    QVector<button> vec_temp_buttons;
    for(auto buttons: vec_blocks)
        for(auto button: buttons)
            vec_temp_buttons.append({button.title, button.next, button.color});
    return vec_temp_buttons;
}

QVector<GraphItem::button> GraphItem::getButtons(int index_block){
    QVector<button> vec_temp_buttons;
    if(index_block > -1 && index_block < vec_blocks.size()){
         for(auto& block_button: vec_blocks[index_block])
            vec_temp_buttons.append({block_button.title, block_button.next, block_button.color});
    }
    return vec_temp_buttons;
}

int GraphItem::getCountBlockButtons(){
    return vec_blocks.count();
}

void GraphItem::setTitleButton(int index, QString title){

}


void GraphItem::setNextItemButton(GraphPoint* point, QGraphicsItem* item){
    for(int i = 0; i < vec_blocks.size(); i++){
        for(int j = 0; j < vec_blocks[i].size(); j++){
            if(vec_blocks[i][j].point == point){
                vec_blocks[i][j].next = item;
                break;
            }
        }
    }
}

void GraphItem::setBlockButtonsStyle(Qt::PenStyle style){
    block_style = style;
}
