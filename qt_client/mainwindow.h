#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QUiLoader>
#include <QHBoxLayout>
#include "src/json.hpp"
#include <QUrlQuery>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHash>
#include <QMessageBox>
#include <QCheckBox>
#include <graphic/graphwidget.h>
#include <QDesktopServices>
#include <QtCharts>
#include <QChartView>
#include <QPieSeries>
#include <QBarSeries>
#include <QGraphicsTextItem>

using namespace nlohmann;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    enum RequestMethod {
        POST,
        GET
    };

    struct parser_task{
        int id;
        QString title;
        QJsonObject data;

        parser_task():id(-1), title(""), data(QJsonObject()){}
        parser_task(int id):id(id), title(""), data(QJsonObject()){}
        parser_task(int id, QString title, QJsonObject data):
            id(id), title(title), data(data){}
        bool operator==(const parser_task &other){
            return id == other.id;
        }
    };

    Ui::MainWindow *ui;
    QWidget* main_widget;
    QHash<QString, QWidget*> hash_widgets;
    QString token;
    QVector<parser_task> vec_tasks;
    QPushButton* active_button;

public:
    MainWindow(QString token, QWidget *parent = nullptr);
    ~MainWindow();

private:
    inline QWidget* loadUI(QString name);
    void insertWidget(QWidget* widget);
    QNetworkReply* createRequest(RequestMethod method, QString url, QString token="", QHash<QString, QString> params=QHash<QString, QString>());


    void listBots(QWidget* widget, QString data);
    QString startBot(int id);
    QString stopBot(int id);
    void deleteBot(int id);
    void commandsWindow(int id, QString commands);
    QPixmap decodeBase64Image(const QString &base64_str);

private slots:
    void exitSystem();
    void menuBots();
    void createBot();
    void replyFinished();
    void setActiveButton();

signals:
    void closeWindow();

};
#endif // MAINWINDOW_H
