QT += core gui designer
QT += network
QT += uitools
QT += charts
QT += printsupport


greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    authwindow.cpp \
    graphic/grapharrow.cpp \
    graphic/graphitem.cpp \
    graphic/graphline.cpp \
    graphic/graphpoint.cpp \
    graphic/graphscene.cpp \
    graphic/graphview.cpp \
    graphic/graphwidget.cpp \
    main.cpp \
    mainwindow.cpp \
    vkauthwindow.cpp

HEADERS += \
    authwindow.h \
    graphic/grapharrow.h \
    graphic/graphitem.h \
    graphic/graphline.h \
    graphic/graphpoint.h \
    graphic/graphscene.h \
    graphic/graphview.h \
    graphic/graphwidget.h \
    mainwindow.h \
    src/json.hpp \
    vkauthwindow.h

FORMS += \
    authwindow.ui \
    graphic/graphwidget.ui \
    mainwindow.ui \
    vkauthwindow.ui \
    vkauthwindow_copy.ui



# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES +=

RESOURCES += \
    resources.qrc




