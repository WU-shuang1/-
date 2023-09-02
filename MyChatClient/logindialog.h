#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class LoginDialog; }
QT_END_NAMESPACE

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
signals:
    //发送登录信号
    void SIG_login(QString tel, QString password);
    //发送注册信号
    void SIG_register(QString tel, QString password);
private slots:
    void on_pb_login_clicked();

    void on_pb_clear_clicked();

    void on_pb_clear_register_clicked();

    void on_pb_register_clicked();

private:
    Ui::LoginDialog *ui;
};
#endif // LOGINDIALOG_H
