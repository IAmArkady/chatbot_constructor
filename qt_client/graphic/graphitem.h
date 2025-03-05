#ifndef GRAPHITEM_H
#define GRAPHITEM_H
#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QApplication>
#include <QTextItem>
#include <QPen>
#include <QGraphicsScene>
#include "graphic/graphpoint.h"



class GraphItem :public QObject, public QGraphicsItem{

    Q_OBJECT
public:
    enum TypeItem {Keywords, Actions, Messages, Requests, Inputs};
    struct button{
        QString title;
        QGraphicsItem* next;
        QColor color;
    };
    struct text_block{
        QString text;
    };

    explicit GraphItem(TypeItem type, QGraphicsItem *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
    QRectF boundingRect() const;
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
    TypeItem getType();
    QVector<GraphPoint*> getPoints();
    void setText(QString text);
    void setTitle(QString text);
    void setTitleColor(QColor color);
    void setNextItem(QGraphicsItem *item);
    QGraphicsItem* getNextItem();
    QColor getTitleColor();
    QString getTitle();

    void addTextBlock(QString text);
    void updateTextBlock(int index, QString text);
    void removeTextBlock(int index);
    void removeAllTextBlock();
    QVector<text_block> getTextBlocks();

    void addButton(int index, QString title, QColor color = Qt::white);
    void updateButton(int index_block, int index_button, QString title, QColor color = Qt::white);
    void addBlockButtons();
    void removeButton(int index_block, int index_button);
    void removeBlockButtons(int index);
    QVector<button> getButtons();
    QVector<button> getButtons(int index_block);
    int getCountBlockButtons();
    void setNextItemButton(GraphPoint *point, QGraphicsItem* item);
    void setBlockButtonsStyle(Qt::PenStyle style);
    void setTitleButton(int index, QString title);

private:
    struct button_graph{
        QString title;
        GraphPoint *point;
        QGraphicsItem* next;
        QColor color;
    };


    QVector<GraphPoint*> vec_points;
    TypeItem type;
    QString title, text, block_title, block_text;
    qreal width_rect;
    QRect rect;
    QColor color_title;
    float boundingRect_width, boundingRect_height;
    QGraphicsItem* next_item;

    QVector<QVector<button_graph>> vec_blocks;
    QVector<text_block> vec_text_blocks;
    Qt::PenStyle block_style;

private slots:
    void slotTriggerPoint();
    void slotDeletePoint(QObject* point);
signals:
    void triggerPoint(GraphPoint* point);
    void deletePoint(GraphPoint* point);
};

#endif // GRAPHITEM_H
