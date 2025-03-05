#include "graphwidget.h"
#include "ui_graphwidget.h"

GraphWidget::GraphWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GraphWidget)
{
    ui->setupUi(this);
    graphScene = new GraphScene(this);

    graphView = new GraphView(this);
    ui->gridLayout->addWidget(graphView);
    graphView->setScene(graphScene);

    viewport_widget = loadUI("/bots/graph/viewport.ui");
    graphView->setViewport(viewport_widget);
    connect(viewport_widget->findChild<QPushButton*>("PB_Save"), &QPushButton::clicked, [this](){
        emit(clickSaveButton());
    });
    connect(viewport_widget->findChild<QPushButton*>("PB_Exit"), &QPushButton::clicked, this, &GraphWidget::deleteLater);
    connect(viewport_widget->findChild<QPushButton*>("PB_Variables"), &QPushButton::clicked, this, &GraphWidget::onClickVariables);

    f_current_widget = false;
    current_item = nullptr;
    count_key = count_act = count_mes = count_req = count_inp = 0;

    hash_color.insert("secondary", QColor(255, 255, 255, 110));
    hash_color.insert("negative", QColor(255, 0, 0, 110));
    hash_color.insert("positive", QColor(0, 255, 0, 110));
    hash_color.insert("primary", QColor(0, 0, 255, 110));

    initUiWidgets();

//    QString expression = "a>b И c<49 ИЛИ c>5 ИЛИ a>5 ИЛИ ABC=34 И c<5";
//    QJsonObject json = expressionToJson(expression);
//    qDebug().noquote() << QJsonDocument(json).toJson(QJsonDocument::Indented);


    connect(graphScene, &GraphScene::selectionChanged, this, &GraphWidget::selectSceneItem);
    connect(graphScene, &GraphScene::addGraphItem, this, &GraphWidget::addSceneGraphItem);
    connect(graphScene, &GraphScene::removeGraphItem, this, &GraphWidget::deleteSceneGraphItem);
}

void GraphWidget::addVariableToList(QListWidget* list_widget, QString name, QString des){
    QWidget* widget_variable = loadUI("bots/graph/variable.ui");
    widget_variable->findChild<QLineEdit*>("LE_Name")->setText(name);
    widget_variable->findChild<QLineEdit*>("LE_Des")->setText(des);

    QListWidgetItem* list_item = new QListWidgetItem(list_widget);
    list_item->setSizeHint(widget_variable->sizeHint());
    list_widget->setItemWidget(list_item, widget_variable);

    QPushButton* del_variable = widget_variable->findChild<QPushButton*>("PB_Delete");
    connect(del_variable, &QPushButton::clicked, [list_widget, list_item, widget_variable]() {
        delete list_widget->takeItem(list_widget->row(list_item));
        delete widget_variable;
    });
}

void GraphWidget::saveVariables(QListWidget* list_widget){
    QVector<variable> vec_tempvar;
    vec_variables.clear();
    for (int i = 0; i < list_widget->count(); i++) {
        QWidget* widget = list_widget->itemWidget(list_widget->item(i));
        if (!widget) continue;

        QString name = widget->findChild<QLineEdit*>("LE_Name")->text();
        QString des = widget->findChild<QLineEdit*>("LE_Des")->text();

        if (!name.isEmpty()){
            if(!vec_tempvar.contains(name))
                vec_tempvar.push_back(variable(name, des));
        }
    }
    vec_variables = vec_tempvar;
}

void GraphWidget::onClickVariables(){
    QDialog* dialog = new QDialog(this);
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    layout->addWidget(loadUI("bots/graph/dialog_variables.ui"));
    QListWidget *list_widget = dialog->findChild<QListWidget*>("LW_Variables");


    for(auto variable: vec_variables)
        addVariableToList(list_widget, variable.name, variable.des);

    connect(dialog->findChild<QPushButton*>("PB_Create"), &QPushButton::clicked, [this, dialog, list_widget](){
        addVariableToList(list_widget);
    });


    connect(dialog, &QDialog::finished, [this, dialog, list_widget](int result){
        saveVariables(list_widget);
        dialog->deleteLater();
    });

    dialog->setWindowTitle("Управление переменными");
    dialog->setMinimumSize(500, 300);
    dialog->exec();
}

void GraphWidget::initUiWidgets(){
    /*Ключевые слова*/
    keywords_widget = loadUI("bots/graph/keywords.ui");
    keywords_widget->findChild<QComboBox*>("CB_Mes")->addItem("Сообщение равно");
    keywords_widget->findChild<QComboBox*>("CB_Mes")->addItem("Сообщение содержит");
    keywords_widget->findChild<QComboBox*>("CB_Mes")->addItem("Сообщение начинается с");
    connect(keywords_widget->findChild<QPushButton*>("PB_Save"), &QPushButton::clicked, [this](){
        current_item->setTitle(keywords_widget->findChild<QLineEdit*>("LE_Title")->text());
        current_item->updateTextBlock(0, keywords_widget->findChild<QComboBox*>("CB_Mes")->currentText()+':');
        current_item->updateTextBlock(1, keywords_widget->findChild<QTextEdit*>("TE_Mes")->toPlainText());
    });

    /*Действия*/
    actions_widget = loadUI("bots/graph/actions.ui");
    connect(actions_widget->findChild<QPushButton*>("PB_AddVar"), &QPushButton::clicked, [this](){
        makeActionsVariable(current_item);
    });

    connect(actions_widget->findChild<QPushButton*>("PB_Save"), &QPushButton::clicked, [this](){
        current_item->setTitle(actions_widget->findChild<QLineEdit*>("LE_Title")->text());
        QLayoutItem* lay_item;
        current_item->removeAllTextBlock();
        for(int i = 0; i < actions_widget->findChild<QVBoxLayout*>("VL_Variables")->count(); i++){
            QString text;
            lay_item = actions_widget->findChild<QVBoxLayout*>("VL_Variables")->itemAt(i);
            text+=lay_item->widget()->findChild<QComboBox*>()->currentText();
            text+=" = ";
            text+=lay_item->widget()->findChild<QLineEdit*>()->text();
            current_item->addTextBlock(text);

        }

    });

    /*Запросы*/
    requests_widget = loadUI("bots/graph/requests.ui");
    requests_widget->findChild<QComboBox*>("CB_Req")->addItem("POST");
    requests_widget->findChild<QComboBox*>("CB_Req")->addItem("GET");
    connect(requests_widget->findChild<QPushButton*>("PB_Save"), &QPushButton::clicked, [this](){
        current_item->setTitle(requests_widget->findChild<QLineEdit*>("LE_Title")->text());
        current_item->updateTextBlock(1, requests_widget->findChild<QComboBox*>("CB_Req")->currentText());
        current_item->updateTextBlock(3, requests_widget->findChild<QLineEdit*>("LE_Link")->text());
        current_item->updateTextBlock(5, requests_widget->findChild<QTextEdit*>("TE_Head")->toPlainText());
        current_item->updateTextBlock(7, requests_widget->findChild<QTextEdit*>("TE_Body")->toPlainText());
        QString current_text = requests_widget->findChild<QComboBox*>("CB_Var")->currentText();
        vec_requests[vec_requests.indexOf(current_item)].variable = current_text;
    });

    /*Ввод*/
    inputs_widget = loadUI("bots/graph/inputs.ui");
    connect(inputs_widget->findChild<QPushButton*>("PB_Save"), &QPushButton::clicked, [this](){
        current_item->setTitle(inputs_widget->findChild<QLineEdit*>("LE_Title")->text());
        current_item->updateTextBlock(1, inputs_widget->findChild<QComboBox*>("CB_Var")->currentText());
    });

    /*Сообщения*/
    messages_widget = loadUI("bots/graph/messages.ui");
    connect(messages_widget->findChild<QPushButton*>("PB_Save"), &QPushButton::clicked, [this](){
        for(int i = 0; i < vec_items_buttons[vec_items_buttons.indexOf(current_item)].vec_buttons.count(); i++)
            current_item->setTitleButton(i, vec_items_buttons[vec_items_buttons.indexOf(current_item)].vec_buttons[i].button->text());

        current_item->setTitle(messages_widget->findChild<QLineEdit*>("LE_Title")->text());
        current_item->updateTextBlock(1, messages_widget->findChild<QTextEdit*>("TE_Mes")->toPlainText());
    });

    connect(messages_widget->findChild<QPushButton*>("PB_AddBlock"), &QPushButton::clicked, [this](){
        QWidget* widget = makeButtonsBlock();
        messages_widget->findChild<QVBoxLayout*>("VL_Buttons")->addWidget(widget);
        current_item->addBlockButtons();
        addButtonBlock(widget);
    });

}

QWidget* GraphWidget::loadUI(QString name){
    QUiLoader loader;
    QFile file(":/forms/"+name);
    file.open(QIODevice::ReadOnly);
    QWidget* widget = loader.load(&file);
    return widget;
}

void GraphWidget::deleteLayout(QLayout* layout){
    while(QLayoutItem* layItem = layout->layout()->takeAt(0)){
        layItem->widget()->deleteLater();
    }
}

QWidget* GraphWidget::makeActionsVariable(GraphItem* graph_item){
    QWidget* widget = new QWidget(this);
    widget->setLayout(new QHBoxLayout(widget));
    QComboBox* box = new QComboBox(widget);
    for(auto var: vec_variables)
        box->addItem(var.name);
    widget->layout()->addWidget(box);
    widget->layout()->addWidget(new QLabel("=", widget));
    widget->layout()->addWidget(new QLineEdit(widget));

    QPushButton* del = new QPushButton(widget);
    del->setIcon(QIcon(":/images/delete.png"));
    widget->layout()->addWidget(del);
    actions_widget->findChild<QVBoxLayout*>("VL_Variables")->addWidget(widget);

    graph_item->addTextBlock(box->currentText() + " = ");
    vec_actions[vec_actions.indexOf(graph_item)].vec_widgets.append(widget);

    connect(del, &QPushButton::clicked, [this, del, graph_item](){
        int index_item = vec_actions.indexOf(graph_item);
        int index_widget = vec_actions[index_item].vec_widgets.indexOf(del->parentWidget());
        vec_actions[index_item].vec_widgets.remove(index_widget);
        graph_item->removeTextBlock(index_widget);
        del->parentWidget()->deleteLater();
    });
    return widget;
}

QWidget* GraphWidget::makeButtonsBlock(){
    QWidget* widget = new QWidget(this);
    widget->setObjectName("block_widget");
    widget->setStyleSheet('#'+widget->objectName()+"{background-color: white; border: 1px solid lightgray; border-radius: 10px}");
    widget->setLayout(new QVBoxLayout(widget));

    QPushButton* button_add = new QPushButton("", widget),
            *button_del = new QPushButton("", widget);
    button_add->setIcon(QIcon(QIcon(":/images/plus.png")));
    button_del->setIcon(QIcon(QIcon(":/images/minus.png")));

    QHBoxLayout* hlayout = new QHBoxLayout(widget);
    hlayout->addSpacing(5);
    button_add->setObjectName("PB_AddButton");
    button_del->setObjectName("PB_Delete");

    hlayout->addStretch();
    hlayout->addWidget(button_add);
    hlayout->addWidget(button_del);

    widget->layout()->addItem(hlayout);

    connect(widget->findChild<QPushButton*>("PB_AddButton"), &QPushButton::clicked, [this, widget](){
        addButtonBlock(widget);
    });

    connect(widget->findChild<QPushButton*>("PB_Delete"), &QPushButton::clicked, [this, widget](){
        int index;
        QList<QPushButton*> list_buttons = widget->findChildren<QPushButton*>("chat_button");
        current_item->removeBlockButtons(messages_widget->findChildren<QWidget*>("block_widget").indexOf(widget));
        for(auto button: list_buttons){
            index = vec_items_buttons[vec_items_buttons.indexOf(current_item)].vec_buttons.indexOf(button);
            if(index > -1){
               vec_items_buttons[vec_items_buttons.indexOf(current_item)].vec_buttons.removeAt(index);
            }
        }
        delete widget;
    });

    return widget;
}

void GraphWidget::addButtonBlock(QWidget* block_widget){
    QPushButton* button = new QPushButton("Кнопка", block_widget);
    button->setObjectName("chat_button");
    block_widget->layout()->addWidget(button);

    int index_block = messages_widget->findChildren<QWidget*>("block_widget").indexOf(block_widget);
    current_item->addButton(index_block, button->text());
    vec_items_buttons[vec_items_buttons.indexOf(current_item)].vec_buttons.append({button, graph_button::Text, "", "secondary"});

    connect(button, &QPushButton::clicked, [this, button](){
        onClickButtonBlock(button);
    });
}

void GraphWidget::onClickButtonBlock(QPushButton* button){
    QPushButton* sender_button = button;
    QDialog* dialog = new QDialog(this);
    QVBoxLayout *layout = new QVBoxLayout(dialog);
    dialog->setFixedHeight(260);
    layout->addWidget(loadUI("bots/graph/dialog_button.ui"));
    dialog->findChild<QLineEdit*>("LE_Title")->setText(sender_button->text());

    QComboBox* combobox = dialog->findChild<QComboBox*>("CB_Type");
    combobox->addItem("Текст");
    combobox->addItem("Ссылка");
    connect(combobox, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, combobox](){
        QLayout* layout_link = combobox->parentWidget()->findChild<QVBoxLayout*>("layout_link"),
                *layout_colors = combobox->parentWidget()->findChild<QVBoxLayout*>("layout_colors");
        switch(combobox->currentIndex()){
        case 0:
            showLayoutContents(layout_colors);
            hideLayoutContents(layout_link);
            break;
        case 1:
            showLayoutContents(layout_link);
            hideLayoutContents(layout_colors);
            break;
        }
    });

    int index = vec_items_buttons[vec_items_buttons.indexOf(current_item)].vec_buttons.indexOf(button);
    switch (vec_items_buttons[vec_items_buttons.indexOf(current_item)].vec_buttons[index].type) {
    case graph_button::Text:
        dialog->findChild<QComboBox*>("CB_Type")->setCurrentIndex(1);
        dialog->findChild<QComboBox*>("CB_Type")->setCurrentIndex(0);
        dialog->findChild<QRadioButton*>(vec_items_buttons[vec_items_buttons.indexOf(current_item)].vec_buttons[index].color)->setChecked(true);
        break;
    case graph_button::Link:
        dialog->findChild<QComboBox*>("CB_Type")->setCurrentIndex(1);
        dialog->findChild<QLineEdit*>("LE_Link")->setText(vec_items_buttons[vec_items_buttons.indexOf(current_item)].vec_buttons[index].link);
        break;
    }

    connect(dialog->findChild<QPushButton*>("PB_Save"), &QPushButton::clicked, [this, dialog, button](){
        onClickDialogSaveButton(dialog, button);
        dialog->close();
    });

    connect(dialog->findChild<QPushButton*>("PB_Delete"), &QPushButton::clicked, [this, dialog, button](){
        onClickDialogDeleteButton(button);
        dialog->close();
    });

    dialog->setLayout(layout);
    dialog->exec();
}

void GraphWidget::onClickDialogSaveButton(QWidget* dialog,QPushButton* button){
    QString title =  dialog->findChild<QLineEdit*>("LE_Title")->text();
    QColor color = hash_color.find("seconadry").value();
    int index_block = messages_widget->findChildren<QWidget*>("block_widget").indexOf(button->parentWidget());
    int index_button = messages_widget->findChildren<QWidget*>("block_widget")[index_block]->findChildren<QPushButton*>("chat_button").indexOf(button);

    int index = vec_items_buttons[vec_items_buttons.indexOf(current_item)].vec_buttons.indexOf(button);
    graph_button* current_button = &vec_items_buttons[vec_items_buttons.indexOf(current_item)].vec_buttons[index];
    current_button->button->setText(dialog->findChild<QLineEdit*>("LE_Title")->text());

    switch (dialog->findChild<QComboBox*>("CB_Type")->currentIndex()) {
    case graph_button::Text:
        for(auto radio_button: dialog->findChildren<QRadioButton*>()){
            if(radio_button->isChecked()){
                QString text_color = colorToString(hash_color.find(radio_button->objectName()).value());
                if(text_color == colorToString(hash_color.find("secondary").value()))
                    text_color = "";
                current_button->button->setStyleSheet("background-color:" + text_color + ';');
                current_button->color = radio_button->objectName();
                current_button->type = graph_button::Text;
                color = hash_color.find(radio_button->objectName()).value();
            }
        }
        break;
    case graph_button::Link:
        current_button->link = dialog->findChild<QLineEdit*>("LE_Link")->text();
        current_button->button->setStyleSheet("");
        current_button->type = graph_button::Link;
        break;
    }

    current_item->updateButton(index_block, index_button, title, color);
}

void GraphWidget::onClickDialogDeleteButton(QPushButton *button){
    int index_block = messages_widget->findChildren<QWidget*>("block_widget").indexOf(button->parentWidget());
    int index_button = messages_widget->findChildren<QWidget*>("block_widget")[index_block]->findChildren<QPushButton*>("chat_button").indexOf(button);
    int index = vec_items_buttons[vec_items_buttons.indexOf(current_item)].vec_buttons.indexOf(button);
    vec_items_buttons[vec_items_buttons.indexOf(current_item)].vec_buttons.removeAt(index);
    delete button;
    if(!messages_widget->findChildren<QWidget*>("block_widget")[index_block]->findChildren<QPushButton*>("chat_button").count()){
        delete messages_widget->findChildren<QWidget*>("block_widget")[index_block];
        current_item->removeBlockButtons(index_block);
    }
    else
        current_item->removeButton(index_block, index_button);
}

QString GraphWidget::colorToString(QColor color){
    QString text_color = "rgba(" + QString::number(color.red()) + ',' + QString::number(color.green()) + ',' + QString::number(color.blue()) + ',' + QString::number(color.alpha()) + ')';
    return text_color;
}


void GraphWidget::selectSceneItem(){    
    QLayoutItem *lay_item;
    QLayout *lay_widget = viewport_widget->findChild<QWidget*>("sideWidget")->layout();
    while((lay_item = lay_widget->layout()->takeAt(0))){
        lay_widget->removeWidget(lay_item->widget());
        lay_item->widget()->setParent(nullptr);
        delete lay_item;
    }
    if (graphScene->selectedItems().isEmpty()){
        return;
    }

    GraphItem* item = qgraphicsitem_cast<GraphItem*>(graphScene->selectedItems().at(0));
    if(graphScene->getGraphItems().indexOf(item) < 0)
        return;

    if(item){
        current_item = item;
        f_current_widget = true;
        QComboBox* box;
        switch(item->getType()){
        case GraphItem::Keywords:{
            keywords_widget->findChild<QLineEdit*>("LE_Title")->setText(item->getTitle());
            keywords_widget->findChild<QComboBox*>("CB_Mes")->setCurrentText(item->getTextBlocks().at(0).text.chopped(1));
            keywords_widget->findChild<QTextEdit*>("TE_Mes")->setText(item->getTextBlocks().at(1).text);
            viewport_widget->findChild<QWidget*>("sideWidget")->layout()->addWidget(keywords_widget);
            break;
        }

        case GraphItem::Actions:{
            actions_widget->findChild<QLineEdit*>("LE_Title")->setText(item->getTitle());
            viewport_widget->findChild<QWidget*>("sideWidget")->layout()->addWidget(actions_widget);
            QLayoutItem* lay_item;
            while((lay_item = actions_widget->findChild<QVBoxLayout*>("VL_Variables")->takeAt(0))){
                lay_item->widget()->setParent(nullptr);
                delete lay_item;
            }
            for(int i = 0; i < vec_actions[vec_actions.indexOf(current_item)].vec_widgets.size(); i++){
                QWidget* widget = vec_actions[vec_actions.indexOf(current_item)].vec_widgets[i];
                box = widget->findChild<QComboBox*>();
                QString current_text = box->currentText();
                box->clear();
                for(auto var: vec_variables)
                    box->addItem(var.name);
                box->setCurrentText(current_text);
                actions_widget->findChild<QVBoxLayout*>("VL_Variables")->addWidget(widget);
            }
            break;
        }

        case GraphItem::Requests:{
            requests_widget->findChild<QLineEdit*>("LE_Title")->setText(item->getTitle());
            requests_widget->findChild<QComboBox*>("CB_Req")->setCurrentText(item->getTextBlocks().at(1).text);
            requests_widget->findChild<QLineEdit*>("LE_Link")->setText(item->getTextBlocks().at(3).text);
            requests_widget->findChild<QTextEdit*>("TE_Head")->setText(item->getTextBlocks().at(5).text);
            requests_widget->findChild<QTextEdit*>("TE_Body")->setText(item->getTextBlocks().at(7).text);
            box = requests_widget->findChild<QComboBox*>("CB_Var");
            box->clear();
            box->addItem("");
            for(auto var: vec_variables)
                box->addItem(var.name);
            box->setCurrentText(vec_requests[vec_requests.indexOf(current_item)].variable);
            viewport_widget->findChild<QWidget*>("sideWidget")->layout()->addWidget(requests_widget);
            break;
        }

        case GraphItem::Inputs:{
            inputs_widget->findChild<QLineEdit*>("LE_Title")->setText(item->getTitle());
            box = inputs_widget->findChild<QComboBox*>("CB_Var");
            box->clear();
            for(auto var: vec_variables)
                box->addItem(var.name);
            box->setCurrentText(item->getTextBlocks().at(1).text);
            viewport_widget->findChild<QWidget*>("sideWidget")->layout()->addWidget(inputs_widget);
            break;
        }

        case GraphItem::Messages:{
            for(int i = 0; i < vec_items_buttons.size(); i++){
                if (vec_items_buttons.at(i).item == item){
                    QLayoutItem* lay_item;
                    while((lay_item = messages_widget->findChild<QVBoxLayout*>("VL_Buttons")->takeAt(0))){
                        lay_item->widget()->setParent(nullptr);
                        delete lay_item;
                    }

                    QVector<QVector<graph_button>> vec_temp_buttons;
                    for(auto but: vec_items_buttons[i].vec_buttons){
                        bool found_group = false;
                         for (auto& vec_temp : vec_temp_buttons) {
                             if (vec_temp.isEmpty() || vec_temp.first().button->parentWidget() == but.button->parentWidget()) {
                                 vec_temp.append(but);
                                 found_group = true;
                                 break;
                             }
                         }
                         if (!found_group) {
                             QVector<graph_button> vec;
                             vec.push_back(but);
                             vec_temp_buttons.push_back(vec);
                         }
                    }

                    for(auto& vec_temp: vec_temp_buttons){
                        QWidget* widget = makeButtonsBlock();
                        for(auto& but: vec_temp)
                            widget->layout()->addWidget(but.button);
                        messages_widget->findChild<QVBoxLayout*>("VL_Buttons")->addWidget(widget);
                    }

                    messages_widget->findChild<QLineEdit*>("LE_Title")->setText(item->getTitle());
                    messages_widget->findChild<QTextEdit*>("TE_Mes")->setText(item->getTextBlocks().at(1).text);
                    viewport_widget->findChild<QWidget*>("sideWidget")->layout()->addWidget(messages_widget);
                }
            }
            break;
        }
        }
    }
}


void GraphWidget::addSceneGraphItem(){
    GraphItem* item = graphScene->getGraphItems().last();
    switch(item->getType()){
    case GraphItem::Keywords:
        item->setTitle("Ключевые слова №" + QString::number(++count_key));
        item->setTitleColor(QColor(240, 230, 150, 200));
        item->addTextBlock("Сообщение равно:");
        item->addTextBlock("");
        break;

    case GraphItem::Messages:        
        item->setTitle("Сообщение №" + QString::number(++count_mes));
        item->setTitleColor(QColor(100, 230, 240, 200));
        vec_items_buttons.append(item);
        item->addTextBlock("Текст сообщения:");
        item->addTextBlock("");
        break;

    case GraphItem::Actions:
        item->setTitle("Действие №" + QString::number(++count_act));
        item->setTitleColor(QColor(240, 150, 150, 200));
        vec_actions.append(item);
        break;

    case GraphItem::Requests:
        item->setTitle("Запрос (JSON) №" + QString::number(++count_req));
        item->setTitleColor(QColor(100, 200, 150, 200));
        item->addTextBlock("Тип запроса:");
        item->addTextBlock("");
        item->addTextBlock("Ссылка:");
        item->addTextBlock("");
        item->addTextBlock("Заголовок:");
        item->addTextBlock("");
        item->addTextBlock("Тело:");
        item->addTextBlock("");
        item->addBlockButtons();
        item->setBlockButtonsStyle(Qt::NoPen);
        item->addButton(0, "Успешно", QColor(0, 255, 0, 100));
        item->addButton(0, "Ошибка", QColor(255, 0, 0, 100));
        item->getPoints().first()->setVisible(false);
        vec_requests.append(item);
        break;

    case GraphItem::Inputs:
        item->setTitle("Ввод №" + QString::number(++count_inp));
        item->setTitleColor(QColor(100, 100, 200, 200));
        item->addTextBlock("Сохранить в переменную:");
        item->addTextBlock("");
        break;
    }

}

void GraphWidget::deleteSceneGraphItem(){

}

QJsonObject GraphWidget::getJson(){
    QVector<GraphItem*> items = graphScene->getGraphItems();
    QJsonArray json_array;

    for(auto item: items){
        QJsonObject json_pos, json_graphItem, json_command;
        QPoint point_pos = item->pos().toPoint();
        json_pos["x"] = point_pos.x();
        json_pos["y"] = point_pos.y();
        json_graphItem["title"] = item->getTitle();
        json_graphItem["pos"] = json_pos;
        json_command["item"] = json_graphItem;
        if(item->getNextItem())
            json_command["next_command"] = items.indexOf(qgraphicsitem_cast<GraphItem*>(item->getNextItem()));
        json_command["command"] = items.indexOf(item);

        if(item->getType() == GraphItem::Keywords){
            json_command["current_index"] = item->getTextBlocks().at(0).text;
            json_command["keywords_text"] = item->getTextBlocks().at(1).text;
            json_command["type_command"] = "keywords";
        }

        else if(item->getType() == GraphItem::Requests){
            json_command["type"] = item->getTextBlocks().at(1).text;
            json_command["link"] = item->getTextBlocks().at(3).text;
            json_command["head"] = item->getTextBlocks().at(5).text;
            json_command["body"] = item->getTextBlocks().at(7).text;
            json_command["save_variable"] = vec_requests[vec_requests.indexOf(item)].variable;

            if(!item->getButtons().isEmpty() && item->getButtons().size()>1){
                QJsonArray json_array_buttons;
                for(int i = 0; i < item->getButtons().size(); i++){
                    QJsonObject json_button;
                    json_button["title"] = item->getButtons().at(i).title;
                    if(item->getButtons().at(i).next)
                        json_button["next_command"] = items.indexOf(qgraphicsitem_cast<GraphItem*>(item->getButtons().at(i).next));;
                    switch(i){
                    case 0:
                        json_button["color"] = "positive";
                        break;
                    case 1:
                        json_button["color"] = "negative";
                        break;
                    }
                    json_array_buttons.push_back(json_button);
                }
                json_command["buttons"] = json_array_buttons;
            }

            json_command["type_command"] = "requests";
        }

        else if (item->getType() == GraphItem::Inputs){
            json_command["variable"] = item->getTextBlocks().at(1).text;
            json_command["type_command"] = "inputs";
        }

        else if (item->getType() == GraphItem::Actions){
            QJsonArray json_array;
            int index = vec_actions.indexOf(item);
            for(auto widget: vec_actions[index].vec_widgets){
                QJsonObject json_object;
                json_object["variable"] = widget->findChild<QComboBox*>()->currentText();
                json_object["value"] = widget->findChild<QLineEdit*>()->text();
                json_array.push_back(json_object);
            }
            json_command["variables"] = json_array;
            json_command["type_command"] = "actions";
        }

        else if(item->getType() == GraphItem::Messages){
            json_command["messages_text"] = item->getTextBlocks().at(1).text;

            if(!item->getButtons().isEmpty()){
                int index_item = vec_items_buttons.indexOf(item);
                QVector<QVector<graph_button>> vec_temp_buttons;
                for(auto but: vec_items_buttons[index_item].vec_buttons){
                    bool found_group = false;
                     for (auto& vec_temp : vec_temp_buttons) {
                         if (vec_temp.isEmpty() || vec_temp.first().button->parentWidget() == but.button->parentWidget()) {
                             vec_temp.append(but);
                             found_group = true;
                             break;
                         }
                     }
                     if (!found_group) {
                         QVector<graph_button> vec;
                         vec.push_back(but);
                         vec_temp_buttons.push_back(vec);
                     }
                }

                QJsonArray  json_array_lines;
                int payload = 0;
                for(int i = 0; i < vec_temp_buttons.size(); i++){
                    QJsonArray json_array_line;
                    for(int j = 0; j < vec_temp_buttons[i].size(); j++){
                        QJsonObject json_button;
                        json_button["title"] = vec_temp_buttons[i][j].button->text();
                        switch(vec_temp_buttons[i][j].type){
                        case graph_button::Text:
                            json_button["type"] = "text";
                            json_button["color"] = vec_temp_buttons[i][j].color;
                            break;
                        case graph_button::Link:
                            json_button["type"] = "link";
                            json_button["link"] = vec_temp_buttons[i][j].link;
                            break;
                        }
                        json_button["payload"] = QString::number(++payload);

                        if(item->getButtons(i).at(j).next)
                            json_button["next_command"] = items.indexOf(qgraphicsitem_cast<GraphItem*>(item->getButtons(i).at(j).next));;
                        json_array_line.push_back(json_button);
                    }
                    json_array_lines.push_back(json_array_line);
                }
                json_command["buttons"] = json_array_lines;
            }
            json_command["type_command"] = "messages";
        }

        json_array.push_back(json_command);
    }

    QJsonArray json_variables;
    for(auto variable: vec_variables){
        QJsonObject json_variable;
        json_variable["title"] = variable.name;
        json_variable["description"] = variable.des;
        json_variables.push_back(json_variable);
    }

    QJsonObject json_main;
    json_main["commands"] = json_array;
    json_main["variables"] = json_variables;
    return json_main;
}

void GraphWidget::setJson(QJsonObject json_object){
    QJsonArray variables_array = json_object.value("variables").toArray();
    foreach(const QJsonValue& variable_value, variables_array){
        QJsonObject variable_object = variable_value.toObject();
        vec_variables.append(variable(variable_object.find("title").value().toString(), variable_object.find("description").value().toString()));
    }

    commands_array = json_object.value("commands").toArray();
    foreach (const QJsonValue& command_value, commands_array) {
        QJsonObject command_object = command_value.toObject();
        createItemFromJson(command_object);
    }
}

void GraphWidget::setCreateNextItem(GraphItem *source_item, GraphPoint *source_point, int next_item){
    if(!hash_created_items.contains(next_item)){
        for (int i = 0; i < commands_array.size(); ++i) {
            QJsonObject object = commands_array.at(i).toObject();
            if (object.value("command").toInt() == next_item) {
                GraphItem* next_command_item = createItemFromJson(object);
                graphScene->createRelationsgGraphItems(source_item, source_point, next_command_item);
                break;
            }
        }
    }
    else{
        GraphItem* next_command_item = hash_created_items.find(next_item).value();
        graphScene->createRelationsgGraphItems(source_item, source_point, next_command_item);
    }
}


GraphItem* GraphWidget::createItemFromJson(QJsonObject json_item){
    GraphItem* current_created_item = nullptr;
    QString type_command = json_item.value("type_command").toString();
    int command = json_item.value("command").toInt();

    QJsonObject item_object = json_item.value("item").toObject();
    int pos_x = item_object.value("pos").toObject().value("x").toInt();
    int pos_y = item_object.value("pos").toObject().value("y").toInt();
    QString title = item_object.value("title").toString();

    if(hash_created_items.contains(command))
        return hash_created_items.find(command).value();

    if(type_command == "keywords"){
        QString current_index = json_item.value("current_index").toString();
        QString keywords_text = json_item.value("keywords_text").toString();

        graphScene->addTypeItem(GraphItem::Keywords);
        current_created_item = graphScene->getGraphItems().last();
        hash_created_items.insert(command, current_created_item);

        current_created_item->setTitle(title);
        current_created_item->updateTextBlock(0, current_index);
        current_created_item->updateTextBlock(1, keywords_text);
        current_created_item->setPos(pos_x, pos_y);

       if(json_item.contains("next_command"))
           setCreateNextItem(current_created_item, current_created_item->getPoints().first(), json_item.value("next_command").toInt());
    }

    else if(type_command == "messages"){
        QString messages_text = json_item.value("messages_text").toString();

        graphScene->addTypeItem(GraphItem::Messages);
        current_created_item = graphScene->getGraphItems().last();
        hash_created_items.insert(command, current_created_item);

        current_created_item->setTitle(title);
        current_created_item->updateTextBlock(1, messages_text);
        current_created_item->setPos(pos_x, pos_y);

        QJsonArray buttons_array = json_item.value("buttons").toArray();
        int index_point = 0;
        if(!buttons_array.isEmpty()){
            for(int i = 0 ; i < buttons_array.size(); i++){
                QJsonArray buttons_lines_array = buttons_array[i].toArray();
                QWidget* parent = new QWidget();
                current_created_item->addBlockButtons();

                for(int j =0; j < buttons_lines_array.size(); j++){
                    ++index_point;
                    QJsonObject button_object = buttons_lines_array[j].toObject();
                    graph_button push_button;
                    QString title_button = button_object.value("title").toString();
                    QString type_button = button_object.value("type").toString();
                    push_button.button = new QPushButton(title_button, parent);
                    push_button.button->setObjectName("chat_button");

                    if(type_button == "text"){
                        QString color_button = button_object.value("color").toString();
                        QString text_color = "";
                        if(color_button != "secondary")
                            text_color = colorToString(hash_color.find(color_button).value());
                        push_button.button->setStyleSheet("background-color:" + text_color + ';');
                        push_button.color = color_button;
                        push_button.type = graph_button::Text;
                    }
                    else if(type_button == "link"){
                        QString link_button = button_object.value("link").toString();
                        push_button.link = link_button;
                        push_button.type = graph_button::Link;
                    }

                    vec_items_buttons[(vec_items_buttons.indexOf(current_created_item))].vec_buttons.append(push_button);
                    QPushButton* clicked_button = push_button.button;
                    connect(clicked_button, &QPushButton::clicked, [this, clicked_button](){
                        onClickButtonBlock(clicked_button);
                    });
                    current_created_item->addButton(i, push_button.button->text(), hash_color.find(push_button.color).value());

                    if(button_object.contains("next_command"))
                        setCreateNextItem(current_created_item, current_created_item->getPoints().at(index_point), button_object.value("next_command").toInt());
                }
            }
        }

        if(json_item.contains("next_command"))
            setCreateNextItem(current_created_item, current_created_item->getPoints().first(), json_item.value("next_command").toInt());
    }

    else if(type_command == "requests"){
        QString type_request = json_item.value("type").toString();
        QString link_request = json_item.value("link").toString();
        QString head_request = json_item.value("head").toString();
        QString body_request = json_item.value("body").toString();
        QString save_variable = json_item.value("save_variable").toString();

        graphScene->addTypeItem(GraphItem::Requests);
        current_created_item = graphScene->getGraphItems().last();
        hash_created_items.insert(command, current_created_item);

        current_created_item->setTitle(title);
        current_created_item->updateTextBlock(1, type_request);
        current_created_item->updateTextBlock(3, link_request);
        current_created_item->updateTextBlock(5, head_request);
        current_created_item->updateTextBlock(7, body_request);
        current_created_item->setPos(pos_x, pos_y);

        vec_requests[vec_requests.indexOf(current_created_item)].variable = save_variable;

        QJsonArray buttons_array = json_item.value("buttons").toArray();
        for(int i = 0 ; i < buttons_array.size(); i++){
            QJsonObject button_object = buttons_array[i].toObject();
            QString title_button = button_object.value("title").toString();
            QString color_button = button_object.value("color").toString();
            if(button_object.contains("next_command"))
                setCreateNextItem(current_created_item, current_created_item->getPoints().at(i+1), button_object.value("next_command").toInt());
        }

        if(json_item.contains("next_command"))
            setCreateNextItem(current_created_item, current_created_item->getPoints().first(), json_item.value("next_command").toInt());
    }

    else if(type_command == "inputs"){
        QString variable_input = json_item.value("variable").toString();
        graphScene->addTypeItem(GraphItem::Inputs);
        current_created_item = graphScene->getGraphItems().last();
        hash_created_items.insert(command, current_created_item);

        current_created_item->setTitle(title);
        current_created_item->updateTextBlock(1, variable_input);
        current_created_item->setPos(pos_x, pos_y);

        if(json_item.contains("next_command"))
            setCreateNextItem(current_created_item, current_created_item->getPoints().first(), json_item.value("next_command").toInt());
    }

    else if(type_command == "actions"){
        QJsonArray json_array = json_item.value("variables").toArray();
        graphScene->addTypeItem(GraphItem::Actions);
        current_created_item = graphScene->getGraphItems().last();
        hash_created_items.insert(command, current_created_item);

        current_created_item->setTitle(title);
        current_created_item->setPos(pos_x, pos_y);
        
        int index = vec_actions.indexOf(current_created_item);
        for(int i = 0; i < json_array.size(); i++){
            QJsonObject json_object = json_array.at(i).toObject();
            QWidget* widget = makeActionsVariable(current_created_item);
            QString var = json_object.value("variable").toString();
            QString val = json_object.value("value").toString();
            widget->findChild<QComboBox*>()->setCurrentText(var);
            widget->findChild<QLineEdit*>()->setText(val);
            QString text = var + " = " + val;
            current_created_item->updateTextBlock(i, text);
        }

        if(json_item.contains("next_command"))
            setCreateNextItem(current_created_item, current_created_item->getPoints().first(), json_item.value("next_command").toInt());
    }

    return current_created_item;
}

void GraphWidget::hideLayoutContents(QLayout* layout) {
    for (int i = 0; i < layout->count(); i++) {
        QLayoutItem* item = layout->itemAt(i);
        QWidget* widget = item->widget();
        if (widget) {
            widget->hide();
        } else {
            QLayout* subLayout = item->layout();
            if (subLayout) {
                hideLayoutContents(subLayout);
            } else {
                QSpacerItem* spacer = item->spacerItem();
                if (spacer) {
                    spacer->changeSize(0, 0);
                }
            }
        }
    }
}

void GraphWidget::showLayoutContents(QLayout* layout) {
    for (int i = 0; i < layout->count(); i++) {
        QLayoutItem* item = layout->itemAt(i);
        QWidget* widget = item->widget();
        if (widget) {
            widget->show();
        } else {
            QLayout* subLayout = item->layout();
            if (subLayout) {
                showLayoutContents(subLayout);
            } else {
                QSpacerItem* spacer = item->spacerItem();
                if (spacer) {
                    spacer->changeSize(spacer->sizeHint().width(), spacer->sizeHint().height());
                }
            }
        }
    }
}


QJsonObject GraphWidget::expressionToJson(QString expression) {
    QStringList subexpressions;
    QJsonArray subconditions;
    QJsonObject condition;

    int orIndex = expression.indexOf("ИЛИ");
    if (orIndex != -1) {
        subexpressions = expression.split("ИЛИ");
        for (QString subexpr : subexpressions) {
            subconditions.append(expressionToJson(subexpr.trimmed()));
        }
        condition.insert("condition", "OR");
        condition.insert("subconditions", subconditions);
    } else {
        int andIndex = expression.indexOf("И");
        if (andIndex != -1) {
            subexpressions = expression.split("И");
            for (QString subexpr : subexpressions) {
                subconditions.append(expressionToJson(subexpr.trimmed()));
            }
            condition.insert("condition", "AND");
            condition.insert("subconditions", subconditions);
        } else {
            QRegularExpression regex("(\\w+)\\s*([><=])\\s*(\\w+)");
            QRegularExpressionMatch match = regex.match(expression);
            if (match.hasMatch()) {
                condition.insert("variable", match.captured(1));
                condition.insert("operator", match.captured(2));
                condition.insert("value", match.captured(3));
            } else {
                qWarning() << "Incorrect expression format:" << expression;
            }
        }
    }

    return condition;
}

GraphWidget::~GraphWidget()
{
    delete ui;
}
