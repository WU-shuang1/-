#include "joindatacard.h"
#include "ui_joindatacard.h"
#include <QMessageBox>

JoinDataCard::JoinDataCard(QWidget *parent) :
    GroupDataCard(parent),
    ui(new Ui::JoinDataCard)
{
    ui->setupUi(this);
}

JoinDataCard::~JoinDataCard()
{
    delete ui;
}
//设置群信息
void JoinDataCard::setGroupInfo(QString name, int groupId, int iconId, int total)
{
    //保存群信息
    m_name = name;
    m_groupId = groupId;
    m_iconId = iconId;
    m_total = total;
    //显示群信息
    ui->lb_setName->setText(m_name);
    ui->lb_setGroupId->setText(QString("%1").arg(m_groupId));
    ui->pb_icon->setIcon(QIcon(QString(":/tx/%1.png").arg(m_iconId)));
    ui->lb_setTotal->setText(QString("%1").arg(m_total));
    this->repaint();
}


void JoinDataCard::on_pb_close_clicked()
{
    this->hide();
}


void JoinDataCard::on_pb_exitGroup_clicked()
{
    if(QMessageBox::Yes == QMessageBox::question(this, "提示", "是否退出该群？")) {
        //发送退出群聊信号给kernel
        Q_EMIT SIG_exitGroup(m_groupId);
    }
}

