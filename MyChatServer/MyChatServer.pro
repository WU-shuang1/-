TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

INCLUDEPATH += \
    ./ \
    ./MySQL \
    ./lock \
    ./mediator \
    ./net \
    ./thread_pool


SOURCES += \
        CKernel.cpp \
        MySQL/CMySql.cpp \
        main.cpp \
        mediator/TcpServerMediator.cpp \
        net/TcpServer.cpp \
        thread_pool/thread_pool.cpp

HEADERS += \
    CKernel.h \
    MySQL/CMySql.h \
    mediator/INetMediator.h \
    mediator/TcpServerMediator.h \
    net/INet.h \
    net/TcpServer.h \
    net/packDef.h \
    thread_pool/thread_pool.h

LIBS += \
    -lpthread \
    -lmysqlclient \
    -lssl \
    -lcrypto
