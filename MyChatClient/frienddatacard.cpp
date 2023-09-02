#include "frienddatacard.h"
#include "ui_frienddatacard.h"
#include <QDebug>
#include <QMessageBox>

FriendDataCard::FriendDataCard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FriendDataCard)
{
    ui->setupUi(this);
    setWindowTitle("好友资料");
}

FriendDataCard::~FriendDataCard()
{
    delete ui;
}
//设置好友信息
void FriendDataCard::setFriendInfo(QString name, QString feeling, int icon, int id)
{
    m_id = id;
    ui->lb_setName->setText(name);
    ui->lb_setFeeling->setText(feeling);
    ui->pb_icon->setIcon(QIcon(QString(":/tx/%1.png").arg(icon)));
    this->repaint();
}

void FriendDataCard::on_pb_deleteFriend_clicked()
{
    qDebug()<<__func__;
    //是否确认删除
    if(QMessageBox::Yes == QMessageBox::question(this, "提示", "是否要删除好友")) {
        //删除好友 通过信号发送给kernel
        this->hide();
        Q_EMIT SIG_deleteFriend(m_id);
    }
}


void FriendDataCard::on_pb_close_clicked()
{
    qDebug()<<__func__;
    this->hide();
}

