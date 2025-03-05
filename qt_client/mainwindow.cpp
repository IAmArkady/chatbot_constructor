#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QString token, QWidget *parent)
    : QMainWindow(parent),
     ui(new Ui::MainWindow),
     main_widget(nullptr),
     active_button(nullptr),
     token(token)
{
    ui->setupUi(this);

    centralWidget()->setStyleSheet("#centralwidget {background-image: url(:/images/prog_back.jpg) 0 0 0 0 stretch stretch;}");

    QWidget* bots = hash_widgets.insert("bots", loadUI("bots/bots.ui")).value();
    connect(bots->findChild<QPushButton*>("PB_CreateBot"), &QPushButton::clicked, this, &MainWindow::createBot);
    connect(ui->PB_Bots, &QPushButton::clicked, this, &MainWindow::menuBots);
    connect(ui->PB_Bots, &QPushButton::clicked, this, &MainWindow::setActiveButton);
    connect(ui->PB_ExitSys, &QPushButton::clicked, this, &MainWindow::exitSystem);
}

void MainWindow::setActiveButton(){
    if (active_button){
        active_button->setProperty("class", "NotActiveButton");
        active_button->setStyle(QApplication::style());
    }
    active_button = qobject_cast<QPushButton*>(sender());
    active_button->setProperty("class", "ActiveButton");
    active_button->setStyle(QApplication::style());
}


QWidget* MainWindow::loadUI(QString name){
    QUiLoader loader;
    QFile file(":/forms/"+name);
    file.open(QIODevice::ReadOnly);
    QWidget* widget = loader.load(&file);
    return widget;
}

void MainWindow::insertWidget(QWidget* widget){
    if (ui->GL_Main->indexOf(widget) < 0){
        if (!ui->GL_Main->count())
            ui->GL_Main->addWidget(widget, 0, 0);
        else{
            QLayoutItem *item = ui->GL_Main->takeAt(0);
            item->widget()->setParent(nullptr);
            delete item;
            ui->GL_Main->addWidget(widget);
        }
    }
}

QPixmap  MainWindow::decodeBase64Image(const QString &base64_str) {
    QString base64_pure = base64_str;
    if (base64_pure.startsWith("data:image"))
        base64_pure = base64_pure.section(',', 1, 1);
    QByteArray image_data = QByteArray::fromBase64(base64_pure.toUtf8());
    QPixmap pixmap;
    pixmap.loadFromData(image_data);
    return pixmap;
}

void MainWindow::menuBots(){
    QWidget* widget = hash_widgets.find("bots").value();
    while(QLayoutItem* lay_item = widget->findChild<QVBoxLayout*>("VL_List")->takeAt(0)){
        delete lay_item->widget();
        delete lay_item;
    }
    insertWidget(widget);
    QNetworkReply* reply = createRequest(GET, "http://127.0.0.1:8000/api/bot/get/", token);
     connect(reply, &QNetworkReply::finished, this, [this, reply, widget](){
         listBots(widget, reply->readAll());
     });
}

void MainWindow::listBots(QWidget *widget, QString data){
    QVector<QWidget*> vec_widget;
    QString temp;
    int id;
    QJsonArray json_groups = QJsonDocument::fromJson(data.toUtf8()).array();
    for (auto it = json_groups.begin(); it != json_groups.end(); ++it) {
        QJsonObject json_group = it->toObject();
        QWidget *widget_bot = loadUI("bots/bot.ui");
        if(json_group.value("status").toBool()){
            widget_bot->findChild<QPushButton*>("PB_Start")->setHidden(true);
            widget_bot->findChild<QPushButton*>("PB_Stop")->setHidden(false);
        }
        else{
            widget_bot->findChild<QPushButton*>("PB_Start")->setHidden(false);
            widget_bot->findChild<QPushButton*>("PB_Stop")->setHidden(true);
        }

        QString commands = json_group.value("commands").toString();
        widget_bot->findChild<QLabel*>("LB_Picture")->setPixmap(decodeBase64Image(json_group.value("photo").toString()).scaled(60, 60, Qt::KeepAspectRatio));
        widget_bot->findChild<QLabel*>("LB_Name")->setText("\""+json_group.value("name").toString()+"\"");
        widget_bot->findChild<QLabel*>("LB_Id")->setText(temp.setNum(id = json_group.value("id").toInt()));
        vec_widget.append(widget_bot);

        connect(widget_bot->findChild<QPushButton*>("PB_Start"), &QPushButton::clicked, [this, id, widget_bot](){
            widget_bot->setCursor(Qt::WaitCursor);
            QJsonObject json_start = QJsonDocument::fromJson(startBot(id).toUtf8()).object();
            if(json_start.contains("success") || json_start.contains("error")){
                widget_bot->findChild<QPushButton*>("PB_Start")->setHidden(true);
                widget_bot->findChild<QPushButton*>("PB_Stop")->setHidden(false);
            }
            else
                QMessageBox::critical(this, "Ошибка", "Ошибка, ответ не был получен");
            widget_bot->setCursor(Qt::ArrowCursor);
        });

        connect(widget_bot->findChild<QPushButton*>("PB_Stop"), &QPushButton::clicked, [this, id, widget_bot](){
            QJsonObject json_stop = QJsonDocument::fromJson(stopBot(id).toUtf8()).object();
            if(json_stop.contains("success") || json_stop.contains("error")){
                widget_bot->findChild<QPushButton*>("PB_Start")->setHidden(false);
                widget_bot->findChild<QPushButton*>("PB_Stop")->setHidden(true);
            }
            else
                QMessageBox::critical(this, "Ошибка", "Ошибка, ответ не был получен");
        });

        connect(widget_bot->findChild<QPushButton*>("PB_Commands"), &QPushButton::clicked, [this, id, commands](){
            commandsWindow(id , commands);
        });

        connect(widget_bot->findChild<QPushButton*>("PB_Delete"), &QPushButton::clicked, [this, id](){
            deleteBot(id);
        });
    }
    for(auto wid: vec_widget)
        widget->findChild<QVBoxLayout*>("VL_List")->addWidget(wid);
    widget->findChild<QVBoxLayout*>("VL_List")->addStretch();
}

QString MainWindow::startBot(int id){
    QHash<QString, QString> params;
    params["id"] = QString::number(id);
    QNetworkReply* reply = createRequest(POST, "http://127.0.0.1:8000/api/bot/start/", token, params);
    while(!reply->isFinished())
        QCoreApplication::processEvents();
   return reply->readAll();
}

QString MainWindow::stopBot(int id){
    QHash<QString, QString> params;
    params["id"] = QString::number(id);
    QNetworkReply* reply = createRequest(POST, "http://127.0.0.1:8000/api/bot/stop/", token, params);
    while(!reply->isFinished())
        QCoreApplication::processEvents();
    return reply->readAll();
}


void MainWindow::commandsWindow(int id, QString commands){
    GraphWidget *graph_widget = new GraphWidget(this);
    ui->gridLayout_4->addWidget(graph_widget, 0, 0);
    QJsonObject json = QJsonDocument::fromJson(commands.toUtf8()).object();
    graph_widget->setJson(json);

    connect(graph_widget, &GraphWidget::clickSaveButton, [this, id, graph_widget](){
        QJsonObject json_object = graph_widget->getJson();
        QHash<QString, QString> params;
        params["id"] = QString::number(id);
        params["commands"] = QJsonDocument(json_object).toJson(QJsonDocument::Compact);

        QNetworkReply* reply = createRequest(POST, "http://127.0.0.1:8000/api/bot/commands/set/", token, params);
        QCoreApplication::processEvents();
        while (!reply->isFinished())
            QCoreApplication::processEvents();
    });

    connect(graph_widget, &GraphWidget::destroyed, [this](){
       menuBots();
    });
}

void MainWindow::deleteBot(int id){
    QHash<QString, QString> params;
    params["id"] = QString::number(id);
    QNetworkReply* reply = createRequest(POST, "http://127.0.0.1:8000/api/bot/delete/", token, params);
    QCoreApplication::processEvents();
    while (!reply->isFinished())
        QCoreApplication::processEvents();
}

void MainWindow::createBot(){
     QNetworkReply* reply = createRequest(POST, "http://127.0.0.1:8000/api/bot/adminGroups/", token);
     connect(reply, &QNetworkReply::finished, this, [this, reply](){
         QDialog *dialog = new QDialog(this);
         dialog->setWindowTitle("Список групп");
         QVBoxLayout* lay = new QVBoxLayout(dialog);
         QString temp;
         json info_groups = json::parse(reply->readAll().toStdString().c_str());
         for (auto it = info_groups.begin(); it != info_groups.end(); ++it) {
             QString id;
             QWidget* widget = loadUI("bots/group.ui");
             widget->findChild<QLabel*>("LB_Picture")->setPixmap(decodeBase64Image(it->at("photo").get<std::string>().c_str()).scaled(40, 40, Qt::KeepAspectRatio));
             widget->findChild<QLabel*>("LB_Name")->setText("\""+QString(it->at("name").get<std::string>().c_str())+"\"");
             widget->findChild<QLabel*>("LB_Id")->setText(temp.setNum(it->at("id").get<std::int32_t>()));
             lay->addWidget(widget);

             id = widget->findChild<QLabel*>("LB_Id")->text();

             connect(widget->findChild<QPushButton*>("PB_VK"), &QPushButton::clicked,[id](){
                 QDesktopServices::openUrl(QUrl("https://vk.com/public"+id));
             });

             connect(widget->findChild<QPushButton*>("PB_Select"), &QPushButton::clicked, [this, id, dialog](){
                 QHash<QString, QString> params;
                 params["id"] =id;
                 QNetworkReply* reply = createRequest(POST, "http://127.0.0.1:8000/api/bot/url/", token, params);
                 connect(reply, &QNetworkReply::finished, [=](){
                    json info = json::parse(reply->readAll().toStdString().c_str());
                    if (info.find("url_oauth") != info.end()){
                        QDesktopServices::openUrl(QUrl(info.at("url_oauth").get<std::string>().c_str()));
                        dialog->close();
                    }
                 });
             });
         }
         dialog->setLayout(lay);
         dialog->setFixedSize(dialog->layout()->sizeHint());
         dialog->exec();
         reply->deleteLater();
     });
}


QNetworkReply* MainWindow::createRequest(RequestMethod method, QString url, QString token, QHash<QString, QString> params){
    QHashIterator<QString, QString> it(params);
    QUrlQuery request_params;
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QUrl qurl(url);
    QNetworkRequest request(qurl);
    QNetworkReply* reply;

    while(it.hasNext()){
        it.next();
        request_params.addQueryItem(it.key(), it.value());
    }

    if (!token.isEmpty())
            request.setRawHeader("Authorization", ("Token " + token).toUtf8());

    switch(method){
        case POST:
            request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
            reply = manager->post(request, request_params.query().toUtf8());
            break;
        case GET:
            qurl.setQuery(request_params);
            reply = manager->get(request);
            break;
    }
    return reply;
}


void MainWindow::replyFinished(){
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    json info = json::parse(reply->readAll().toStdString());
    if(info.find("error") != info.end()){
        qDebug() << info.at("error").find("error_code")->get<std::int32_t>();
        qDebug() << info.at("error").find("error_msg")->get<std::string>().c_str();
    }
}


void MainWindow::exitSystem(){
    emit closeWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

