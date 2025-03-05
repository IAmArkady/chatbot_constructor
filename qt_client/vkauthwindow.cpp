#include "vkauthwindow.h"
#include "ui_vkauthwindow.h"

VkAuthWindow::VkAuthWindow(QWidget *parent) :
    QMainWindow(parent), mainWindow(nullptr), ui(new Ui::VkAuthWindow), show_pass(false)
{
    ui->setupUi(this);
    setFixedSize(430, 650);
    setWindowIcon(QIcon(":/images/vk.png"));
    centralWidget()->setStyleSheet("#centralwidget {border-image: url(:/images/auth_back.jpg) 0 0 0 0 stretch stretch;}");

    ui->pushButton_Back->setHidden(true);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    ui_file.setFileName(":/forms/auth/auth.ui");
    ui_widget = ui_loader.load(&ui_file);
    ui_file.close();
    vbox->addWidget(ui_widget);
    ui->verticalLayout_3->addLayout(vbox);

    filename = "data.bin";
    token = readFile(filename);
    connect(ui_widget->findChild<QPushButton*>("PB_Authorization"), &QPushButton::clicked, this, &VkAuthWindow::auth);
    connect(ui->pushButton_Back, &QPushButton::clicked, this, &VkAuthWindow::reShow);
}

void VkAuthWindow::show(){
    if(!token.isEmpty()){
        mainWindow = new MainWindow(token);
        mainWindow->show();
        connect(mainWindow, &MainWindow::closeWindow, this, &VkAuthWindow::replyLogout);
    }
    else
        QWidget::show();
}


void VkAuthWindow::auth(){
    qobject_cast<QPushButton*>(sender())->setEnabled(false);
    ui_widget->findChild<QLabel*>("L_Error")->clear();
    QApplication::setOverrideCursor(Qt::WaitCursor);
    QHash<QString, QString> params;
    QNetworkReply* reply = createPost("http://127.0.0.1:8000/api/auth/url/", "", params);
    connect(reply, &QNetworkReply::finished, this, &VkAuthWindow::replyAuth);
}


void VkAuthWindow::replyAuth(){
    QApplication::restoreOverrideCursor();
    ui_widget->findChild<QPushButton*>("PB_Authorization")->setEnabled(true);
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (reply->size() == 0){
       ui_widget->findChild<QLabel*>("L_Error")->setText("Ошибка соединения. Ответ не был получен.");
       return;
    }
    QString result = reply->readAll();
    QJsonObject json_url = QJsonDocument::fromJson(result.toUtf8()).object();
    if (json_url.contains("url_oauth")){
        QDesktopServices::openUrl(QUrl(json_url.value("url_oauth").toString()));

        QTime stop_time= QTime::currentTime().addSecs(1);
        while (QTime::currentTime() < stop_time)
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

        QVBoxLayout* vbox_temp =  ui->verticalLayout_3->findChildren<QVBoxLayout* >().first();
        QLayoutItem *item = vbox_temp->takeAt(0);
        item->widget()->setParent(nullptr);
        delete vbox_temp;
        delete item;

        QVBoxLayout *vbox = new QVBoxLayout(this);
        ui_file.setFileName(":/forms/auth/enter.ui");
        ui_widget = ui_loader.load(&ui_file);
        ui_file.close();
        vbox->addWidget(ui_widget);
        ui->verticalLayout_3->addLayout(vbox);

        ui->pushButton_Back->setHidden(false);

        QAction* action = ui_widget->findChild<QLineEdit*>("LE_Code")->addAction(QIcon(":/images/eye_open.png"), QLineEdit::TrailingPosition);
        button = qobject_cast<QToolButton *>(action->associatedWidgets().last());
        button->setCursor(QCursor(Qt::PointingHandCursor));
        connect(button, &QToolButton::clicked, this, &VkAuthWindow::changeIconPass);
        connect(ui_widget->findChild<QPushButton*>("PB_Enter"), &QPushButton::clicked, this, &VkAuthWindow::enter);
        connect(ui_widget->findChild<QCheckBox*>("CB_Remember"), &QCheckBox::clicked, this, &VkAuthWindow::clickCheckBox);
    }
    else
        ui_widget->findChild<QLabel*>("L_Error")->setText("Ошибка перенаправления на страницу входа!");
}


void VkAuthWindow::changeIconPass(){
    show_pass = !show_pass;
    if(show_pass){
        button->setIcon(QIcon(":/images/eye_close.png"));
        ui_widget->findChild<QLineEdit*>("LE_Code")->setEchoMode(QLineEdit::Normal);
    }
    else{
        button->setIcon(QIcon(":/images/eye_open.png"));
        ui_widget->findChild<QLineEdit*>("LE_Code")->setEchoMode(QLineEdit::Password);
    }
}

void VkAuthWindow::enter(){
    ui_widget->findChild<QLabel*>("L_Error")->clear();
    QHash<QString, QString> params;
    params["code"] = ui_widget->findChild<QLineEdit*>("LE_Code")->text();
    QNetworkReply* reply = createPost("http://127.0.0.1:8000/api/auth/login/", "", params);
    connect(reply, &QNetworkReply::finished, this, &VkAuthWindow::replyEnter);
}


void VkAuthWindow::replyEnter(){
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    QString result = reply->readAll();
    QJsonObject json_auth = QJsonDocument::fromJson(result.toUtf8()).object();
    if(json_auth.contains("token")){
        token =  json_auth.value("token").toString();
        if(ui_widget->findChild<QCheckBox*>("CB_Remember")->isChecked())
            saveFile(filename, token);
        mainWindow = new MainWindow(token);
        mainWindow->show();
        close();
        connect(mainWindow, &MainWindow::closeWindow, this, &VkAuthWindow::replyLogout);
    }
    else if(json_auth.contains("error_msg"))
        ui_widget->findChild<QLabel*>("L_Error")->setText("Ошибка авторизации, неверный код!");
    else
        ui_widget->findChild<QLabel*>("L_Error")->setText("Ошибка авторизации");
}

void VkAuthWindow::replyLogout(){
    QNetworkReply* reply = createPost("http://127.0.0.1:8000/api/auth/logout/", token);
    connect(reply, &QNetworkReply::finished, [=](){
        qDebug() << reply->errorString();
    });
    token.clear();
    removeFile(filename);
    reShow();
}

void VkAuthWindow::reShow(){
    if(mainWindow){
        mainWindow->close();
        delete mainWindow;
    }

    ui->pushButton_Back->setHidden(true);

    QVBoxLayout* vbox_temp =  ui->verticalLayout_3->findChildren<QVBoxLayout* >().first();
    QLayoutItem *item = vbox_temp->takeAt(0);
    item->widget()->setParent(nullptr);
    delete vbox_temp;
    delete item;

    QVBoxLayout *vbox = new QVBoxLayout(this);
    ui_file.setFileName(":/forms/auth/auth.ui");
    ui_widget = ui_loader.load(&ui_file);
    ui_file.close();
    vbox->addWidget(ui_widget);
    ui->verticalLayout_3->addLayout(vbox);
    connect(ui_widget->findChild<QPushButton*>("PB_Authorization"), &QPushButton::clicked, this, &VkAuthWindow::auth);

    show();
}


QNetworkReply* VkAuthWindow::createPost(QString url, QString token, QHash<QString, QString> params){
    QHashIterator<QString, QString> it(params);
    QUrlQuery post_params;
    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    while(it.hasNext()){
        it.next();
        post_params.addQueryItem(it.key(), it.value());
    }
    if (!token.isEmpty())
            request.setRawHeader("Authorization", ("Token " + token).toUtf8());
    QNetworkReply* reply = manager->post(request, post_params.query().toUtf8());
    return reply;
}

void VkAuthWindow::saveFile(QString filename, QString text){
    QFile file(filename);
    file.setFileName("data.bin");
    file.open(QIODevice::WriteOnly);
    QDataStream in(&file);
    in << text;
    file.close();
}

QString VkAuthWindow::readFile(QString filename){
    QFile file(filename);
    QString text = "";
    file.setFileName("data.bin");
    if(file.open(QIODevice::ReadOnly)){
        QDataStream out(&file);
        out >> text;
        file.close();
    }
    return text;
}

void VkAuthWindow::removeFile(QString filename){
    QFile file(filename);
    file.setFileName("data.bin");
    if(file.open(QIODevice::ReadOnly))
        file.remove();
}

void VkAuthWindow::clickCheckBox(){
    if(!ui_widget->findChild<QCheckBox*>("CB_Remember")->isChecked())
        removeFile(filename);
}

VkAuthWindow::~VkAuthWindow()
{
    delete ui;
}
