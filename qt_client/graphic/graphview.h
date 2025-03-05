#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H
#pragma once
#include <QGraphicsView>
#include <QMouseEvent>
#include <QObject>
#include <QMenu>
#include <QDebug>
#include <QApplication>
#include <math.h>
#include <QGraphicsProxyWidget>
#include "graphic/graphscene.h"
#include "graphic/graphitem.h"
#include "graphic/grapharrow.h"

class GraphView : public QGraphicsView
{
    Q_OBJECT
    QPoint clickPos;

    bool viewportHoverWidget();
public:
    GraphView(QWidget* parent = nullptr);

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void drawBackground(QPainter* painter, const QRectF& rect) override;

public slots:
    void triggerMenu(QAction*);
    void triggerDelMenu(QAction* action);

};

#endif // GRAPHVIEW_H
