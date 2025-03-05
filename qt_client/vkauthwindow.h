#ifndef VKAUTHWINDOW_H
#define VKAUTHWINDOW_H

#include <QPushButton>
#include <QMainWindow>
#include <QDebug>
#include <QToolButton>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QCheckBox>
#include <QAction>
#include <QDesktopServices>
#include <QLineEdit>
#include <mainwindow.h>

namespace Ui {
class VkAuthWindow;
}
using namespace nlohmann;
class VkAuthWindow : public QMainWindow
{
    Q_OBJECT
    MainWindow* mainWindow;
    Ui::VkAuthWindow *ui;
    QToolButton* button;
    QFile ui_file;
    QUiLoader ui_loader;
    QWidget* ui_widget;
    QString token, filename;
    bool show_pass;
public:
    explicit VkAuthWindow(QWidget *parent = nullptr);
    ~VkAuthWindow();
     virtual void show();

private:

    void saveFile(QString filename, QString text);
    QString readFile(QString filename);
    void removeFile(QString filename);
    QNetworkReply* createPost(QString url, QString token, QHash<QString, QString> params=QHash<QString, QString>());

private slots:
    void changeIconPass();
    void clickCheckBox();
    void auth();
    void enter();
    void replyAuth();
    void replyLogout();
    void replyEnter();
    void reShow();
};

#endif // VKAUTHWINDOW_H
