QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    chatdialog.cpp \
    ckernel.cpp \
    createdatacard.cpp \
    fileprocessitem.cpp \
    frienddatacard.cpp \
    groupchatdialog.cpp \
    groupchatitem.cpp \
    groupdatacard.cpp \
    groupitem.cpp \
    joindatacard.cpp \
    main.cpp \
    logindialog.cpp \
    mediator/INetMediator.cpp \
    mediator/TcpClientMediator.cpp \
    mychatdialog.cpp \
    net/TcpClient.cpp \
    userdatacard.cpp \
    useritem.cpp

HEADERS += \
    chatdialog.h \
    ckernel.h \
    createdatacard.h \
    fileprocessitem.h \
    frienddatacard.h \
    groupchatdialog.h \
    groupchatitem.h \
    groupdatacard.h \
    groupitem.h \
    joindatacard.h \
    logindialog.h \
    mediator/INetMediator.h \
    mediator/TcpClientMediator.h \
    mychatdialog.h \
    net/INet.h \
    net/TcpClient.h \
    net/packDef.h \
    userdatacard.h \
    useritem.h

INCLUDEPATH += \
    ./net \
    ./mediator

LIBS += \
    -lws2_32

FORMS += \
    chatdialog.ui \
    createdatacard.ui \
    fileprocessitem.ui \
    frienddatacard.ui \
    groupchatdialog.ui \
    groupchatitem.ui \
    groupitem.ui \
    joindatacard.ui \
    logindialog.ui \
    mychatdialog.ui \
    userdatacard.ui \
    useritem.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
