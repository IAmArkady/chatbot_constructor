#include "authwindow.h"
#include "ui_authwindow.h"

AuthWindow::AuthWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AuthWindow)
{
    ui->setupUi(this);
    connect(ui->PB_Enter, &QPushButton::clicked, this, &AuthWindow::enter);
    connect(ui->PB_Exit, &QPushButton::clicked, qApp, exit);
}

void AuthWindow::enter(){
    manager = new QNetworkAccessManager();
    QNetworkRequest request(QUrl("http://127.0.0.1:8000/auth/token/login/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");
    QUrlQuery params;
    params.addQueryItem("username", ui->LE_Log->text());
    params.addQueryItem("password", ui->LE_Pass->text());
    reply = manager->post(request, params.query().toUtf8());
    connect(reply, &QNetworkReply::finished, this, &AuthWindow::replyFinished);
}

void AuthWindow::replyFinished(){
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    json info = json::parse(reply->readAll().toStdString());
    if(info.find("auth_token") != info.end()){
        qDebug() << info.at("auth_token").get<std::string>().c_str()<<endl;
        setVisible(false);
    }
    else
        if(info.find("non_field_errors") != info.end()){
           ui->L_Error->setText(info.at("non_field_errors").at(0).get<std::string>().c_str());
           ui->LE_Log->setStyleSheet(ui->LE_Log->styleSheet().replace("border: 2px solid black;", "border: 2px solid red;"));
           ui->LE_Pass->setStyleSheet(ui->LE_Log->styleSheet().replace("border: 2px solid black;", "border: 2px solid red;"));
           ui->LE_Pass->clear();
        }
}

AuthWindow::~AuthWindow()
{
    delete ui;
}
