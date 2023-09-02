#include "userdatacard.h"
#include "ui_userdatacard.h"
#include <QMessageBox>

UserDataCard::UserDataCard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserDataCard)
{
    ui->setupUi(this);
    setWindowTitle("个人资料");
}

UserDataCard::~UserDataCard()
{
    delete ui;
}
//设置用户信息
void UserDataCard::SetUserdata(QString name, QString feeling, int iconId)
{
    m_name = name;
    m_feeling = feeling;
    m_iconId = iconId;
    m_tempIconId = m_iconId;

    ui->le_name->setText(m_name);
    ui->le_feeling->setText(m_feeling);
    ui->pb_icon->setIcon(QIcon(QString(":/tx/%1.png").arg(m_tempIconId)));
    this->repaint();
}

void UserDataCard::on_pb_prev_clicked()
{
    m_tempIconId--;
    if(m_tempIconId < 0) m_tempIconId = 35;

    ui->pb_icon->setIcon(QIcon(QString(":/tx/%1.png").arg(m_tempIconId)));
    this->repaint();
}


void UserDataCard::on_pb_next_clicked()
{
    m_tempIconId++;
    if(m_tempIconId > 35) m_tempIconId = 0;
    ui->pb_icon->setIcon(QIcon(QString(":/tx/%1.png").arg(m_tempIconId)));
    this->repaint();
}


void UserDataCard::on_pb_close_clicked()
{
    ui->le_name->setText(m_name);
    ui->le_feeling->setText(m_feeling);
    ui->pb_icon->setIcon(QIcon(QString(":/tx/%1.png").arg(m_iconId)));
    this->repaint();
    this->hide();
}


void UserDataCard::on_pb_save_clicked()
{
    if(QMessageBox::Yes == QMessageBox::question(this, "提示", "是否保存资料")) {
        if(m_iconId == m_tempIconId && m_name == ui->le_name->text() && m_feeling == ui->le_feeling->text()) {
            //如果没发生变化 直接退出
            this->hide();
            return;
        }

        //保存当前窗口的信息
        m_iconId = m_tempIconId;
        m_name = ui->le_name->text();
        m_feeling = ui->le_feeling->text();
        this->hide();

        //将保存的信息通过信号发送给kernel
        Q_EMIT SIG_saveUserData(m_name, m_feeling, m_iconId);
    }

}

