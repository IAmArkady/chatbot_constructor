#ifndef GRAPHWIDGET_H
#define GRAPHWIDGET_H
#pragma once
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDebug>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QUiLoader>
#include <QDebug>
#include <QLineEdit>
#include <QGraphicsProxyWidget>
#include <QToolButton>
#include <QDialog>
#include <QRadioButton>
#include <QHash>
#include <QSpacerItem>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QListWidget>
#include <QScrollArea>
#include <QLabel>
#include "graphic/graphscene.h"
#include "graphic/graphview.h"
#include "graphic/graphitem.h"

namespace Ui {
class GraphWidget;
}

class GraphWidget : public QWidget
{
    Q_OBJECT
    struct graph_button{
        enum typeButton{Text, Link};
        QPushButton* button;
        typeButton type;
        QString link;
        QString color;

        graph_button(){
            button = nullptr;
        }

        graph_button(QPushButton* other_button){
            button = other_button;
        }

        graph_button(QPushButton* other_button, typeButton other_type, QString other_link, QString other_color){
            button = other_button;
            type = other_type;
            link = other_link;
            color = other_color;
        }

        bool operator==(const graph_button& other){
            if(other.button == button)
                return true;
            return false;
        }
    };

    struct variable{
        QString name;
        QString des;

        variable() = default;
        variable(QString name): name(name){}
        variable(QString name, QString des): name(name), des(des){}

        bool operator==(const QString &other_name) const{
            return name == other_name;
        }

        bool operator==(const variable &other) const {
            return name == other.name;
        }
    };

    struct GraphItemBase {
        GraphItem* item = nullptr;

        GraphItemBase() = default;
        GraphItemBase(GraphItem* other_item) : item(other_item) {}

        bool operator==(const GraphItemBase& other) const {
            return item == other.item;
        }
    };


    struct item_buttons : public GraphItemBase {
        QVector<graph_button> vec_buttons;

        using GraphItemBase::GraphItemBase;
    };

    struct action_widgets : public GraphItemBase{
        QVector<QWidget*> vec_widgets;

        using GraphItemBase::GraphItemBase;
        action_widgets(GraphItem* other_item, QVector<QWidget*> other_vec_widgets)
            : GraphItemBase(other_item), vec_widgets(std::move(other_vec_widgets)) {}
    };

    struct request_variable : public GraphItemBase {
        QString variable;

        using GraphItemBase::GraphItemBase;
        request_variable(GraphItem* other_item, QString other_variable)
            : GraphItemBase(other_item), variable(std::move(other_variable)) {}
    };


    QWidget *viewport_widget, *keywords_widget, *messages_widget, *requests_widget, *inputs_widget, *actions_widget;
    GraphItem* current_item;
    GraphView* graphView;
    GraphScene* graphScene;
    QVector<item_buttons> vec_items_buttons;
    QHash<QString, QColor> hash_color;
    QHash<int, GraphItem*> hash_created_items;

    QPushButton* saveButton;
    bool f_current_widget;
    int current_index;
    int count_key, count_act, count_mes, count_req, count_inp;

    QJsonArray commands_array;

    QVector<variable> vec_variables;
    QVector<action_widgets> vec_actions;
    QVector<request_variable> vec_requests;


public:
    explicit GraphWidget(QWidget *parent = nullptr);
    ~GraphWidget();
    QJsonObject getJson();
    void setJson(QJsonObject json_object);

private:
    Ui::GraphWidget *ui;
    void deleteLayout(QLayout* layout);
    inline QWidget* loadUI(QString name);
    void initUiWidgets();
    QWidget* makeButtonsBlock();
    QWidget* makeActionsVariable(GraphItem* graph_item);
    QString colorToString(QColor color);
    void hideLayoutContents(QLayout* layout);
    void showLayoutContents(QLayout* layout);
    QJsonObject expressionToJson(QString expression);
    GraphItem* createItemFromJson(QJsonObject json_item);

    void addButtonBlock(QWidget* block_widget);
    void onClickButtonBlock(QPushButton* button);
    void onClickDialogSaveButton(QWidget* dialog, QPushButton* button);
    void onClickDialogDeleteButton(QPushButton* button);
    void setCreateNextItem(GraphItem* source_item, GraphPoint* source_point, int next_item);

private slots:
    void selectSceneItem();
    void addSceneGraphItem();
    void deleteSceneGraphItem();
    void onClickVariables();

    void addVariableToList(QListWidget* list_widget, QString name = "", QString des = "");
    void saveVariables(QListWidget* list_widget);

signals:
    void clickSaveButton();
};

#endif // GRAPHWIDGET_H
