#include "createdatacard.h"
#include "ui_createdatacard.h"
#include <QMessageBox>

CreateDataCard::CreateDataCard(QWidget *parent) :
    GroupDataCard(parent),
    ui(new Ui::CreateDataCard)
{
    ui->setupUi(this);
}

CreateDataCard::~CreateDataCard()
{
    delete ui;
}
//设置群信息
void CreateDataCard::setGroupInfo(QString name, int groupId, int iconId, int total)
{
    //保存信息
    m_name = name;
    m_groupId = groupId;
    m_iconId = iconId;
    m_total = total;
    m_tempIconId = m_iconId;
    //显示信息
    ui->le_name->setText(m_name);
    ui->lb_setGroupId->setText(QString("%1").arg(m_groupId));
    ui->pb_icon->setIcon(QIcon(QString(":/tx/%1.png").arg(m_iconId)));
    ui->lb_setTotal->setText(QString("%1").arg(m_total));
    this->repaint();
}



void CreateDataCard::on_pb_deleteGroup_clicked()
{
    if(QMessageBox::Yes == QMessageBox::question(this, "提示", "是否解散该群？")) {
        //发送信号给kernel
        Q_EMIT SIG_deleteGroup(m_groupId);
    }
}


void CreateDataCard::on_pb_save_clicked()
{
    if(QMessageBox::Yes == QMessageBox::question(this, "提示", "是否保存群资料？")) {
        if(m_name == ui->le_name->text() && m_iconId == m_tempIconId) {
            //关闭窗口
            this->hide();
            return;
        }
        //保存信息
        m_name = ui->le_name->text();
        m_iconId = m_tempIconId;
        //显示信息
        ui->le_name->setText(m_name);
        ui->pb_icon->setIcon(QIcon(QString(":/tx/%1.png").arg(m_iconId)));
        this->repaint();
        //关闭窗口
        this->hide();

        //发送信号给kernel
        Q_EMIT SIG_updataGroupInfo(m_name, m_groupId, m_iconId);
    }
}


void CreateDataCard::on_pb_close_clicked()
{
    //还原信息
    m_tempIconId = m_iconId;
    ui->le_name->setText(m_name);
    ui->pb_icon->setIcon(QIcon(QString(":/tx/%1.png").arg(m_iconId)));
    this->repaint();
    //关闭窗口
    this->hide();
}


void CreateDataCard::on_pb_prev_clicked()
{
    m_tempIconId--;
    if(m_tempIconId<0) m_tempIconId = 35;
    ui->pb_icon->setIcon(QIcon(QString(":/tx/%1.png").arg(m_tempIconId)));
    this->repaint();
}


void CreateDataCard::on_pb_next_clicked()
{
    m_tempIconId++;
    if(m_tempIconId>35) m_tempIconId = 0;
    ui->pb_icon->setIcon(QIcon(QString(":/tx/%1.png").arg(m_tempIconId)));
    this->repaint();
}

