#ifndef AUTHWINDOW_H
#define AUTHWINDOW_H

#include <QMainWindow>
#include <mainwindow.h>
#include "src/json.hpp"

using namespace nlohmann;

namespace Ui {
class AuthWindow;
}

class AuthWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AuthWindow(QWidget *parent = nullptr);
    ~AuthWindow();

private:
    Ui::AuthWindow *ui;
    MainWindow* mainWindow;
    QNetworkAccessManager* manager;
    QNetworkReply* reply;

private slots:
    void enter();
    void replyFinished();
};

#endif // AUTHWINDOW_H
