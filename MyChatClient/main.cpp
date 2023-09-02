//#include "logindialog.h"
#include "ckernel.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CKernel kernel;
//    LoginDialog w;
//    w.show();
    return a.exec();
}
