#include "logindialog.h"
#include "ui_logindialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QRegularExpression>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    //设置窗口标题
    this->setWindowTitle("聊天APP");
}

LoginDialog::~LoginDialog()
{
    delete ui;
}


void LoginDialog::on_pb_login_clicked()
{
    qDebug()<<__func__;
    //登录前先进行本地校验

    //1.检测手机号是否合法
    QString strTel = ui->le_tel->text();
    QString strTelTemp = strTel;


    if(strTel.isEmpty() || strTelTemp.remove(" ").isEmpty()) {
        QMessageBox::about(this, "提示", "手机号不能为空，请重新输入");
        return;
    }
    //使用正则表达式匹配手机号
    //以1位首第二位为3-9 第三位到第11位为0-9
    QString pattern = "^1[3-9][0-9]{9}$";
    QRegularExpression regex(pattern);
    QRegularExpressionMatch match = regex.match(strTel);
    if(!match.hasMatch()){
        QMessageBox::about(this, "提示", "手机号不合法，请重新输入");
        return;
    }

    //2.检测密码是否合法
    QString strPassword = ui->le_password->text();
    QString strPasswordTemp = strPassword;
    if(strPassword.isEmpty() || strPasswordTemp.remove(" ").isEmpty()) {
        QMessageBox::about(this, "提示", "密码不不能为空，请重新输入");
        return;
    }
    //密码不能超过15位
    if(strPassword.size() > 15) {
        QMessageBox::about(this, "提示", "密码不合法，请重新输入");
        return;
    }

    //密码中不能含有空格
    if(strPassword.size() != strPasswordTemp.size()) {
        QMessageBox::about(this, "提示", "密码不合法，请重新输入");
        return;
    }



    //将账号密码发送给kernel
    Q_EMIT SIG_login(strTel, strPassword);
}


void LoginDialog::on_pb_clear_clicked()
{
    qDebug()<<__func__;
    //清空编辑栏内容
    ui->le_password->setText("");
    ui->le_tel->setText("");
}


void LoginDialog::on_pb_clear_register_clicked()
{
    qDebug()<<__func__;
    //清空编辑栏内容
    ui->le_tel_register->setText("");
    ui->le_password_register->setText("");
    ui->le_ispassword->setText("");
}


void LoginDialog::on_pb_register_clicked()
{
    qDebug()<<__func__;
    //注册前先进行本地校验

    //1.检测手机号是否合法
    QString strTel = ui->le_tel_register->text();
    QString strTelTemp = strTel;


    if(strTel.isEmpty() || strTelTemp.remove(" ").isEmpty()) {
        QMessageBox::about(this, "提示", "手机号不能为空，请重新输入");
        return;
    }
    //使用正则表达式匹配手机号
    //以1位首第二位为3-9 第三位到第11位为0-9
    QString pattern = "^1[3-9][0-9]{9}$";
    QRegularExpression regex(pattern);
    QRegularExpressionMatch match = regex.match(strTel);
    if(!match.hasMatch()){
        QMessageBox::about(this, "提示", "手机号不合法，请重新输入");
        return;
    }

    //2.检测密码是否合法
    QString strPassword = ui->le_password_register->text();
    QString strPasswordTemp = strPassword;
    if(strPassword.isEmpty() || strPasswordTemp.remove(" ").isEmpty()) {
        QMessageBox::about(this, "提示", "密码不能为空，请重新输入");
        return;
    }
    //密码不能超过15位
    if(strPassword.size() > 15) {
        QMessageBox::about(this, "提示", "密码不合法，请重新输入");
        return;
    }

    //密码中不能含有空格
    if(strPassword.size() != strPasswordTemp.size()) {
        QMessageBox::about(this, "提示", "密码不合法，请重新输入");
        return;
    }

    //检测确认密码与密码相同
    QString strIsPassword = ui->le_ispassword->text();
    if(strIsPassword != strPassword) {
        QMessageBox::about(this, "提示", "两次输入密码不一致，请重新输入");
        return;
    }

    //将账号密码发送给kernel
    Q_EMIT SIG_register(strTel, strPassword);
}

