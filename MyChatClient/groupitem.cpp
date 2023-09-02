#include "groupitem.h"
#include "ui_groupitem.h"
#include "createdatacard.h"
#include "joindatacard.h"

GroupItem::GroupItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GroupItem)
{
    ui->setupUi(this);
    m_pDataCard = nullptr;
}

GroupItem::~GroupItem()
{
    delete ui;
    if(m_pDataCard) {
        m_pDataCard->hide();
        delete m_pDataCard;
        m_pDataCard = nullptr;
    }
}
//设置群信息
void GroupItem::setGroupInfo(QString name, int iconId, int total, int groupId)
{
    m_name = name;
    m_iconId = iconId;
    m_groupId = groupId;
    ui->lb_name->setText(m_name);
    ui->pb_icon->setIcon(QIcon(QString(":/tx/%1.png").arg(m_iconId)));

    m_pDataCard->setGroupInfo(name, groupId, iconId, total);
    this->repaint();
}

void GroupItem::on_pb_icon_clicked()
{
    //显示群聊资料
    //获取鼠标位置
    QPoint pos = QCursor::pos();
    m_pDataCard->showNormal();
    int x = pos.x() - m_pDataCard->width();
    if(x<0) x=0;
    m_pDataCard->move(QPoint(x,pos.y()) );
}


void GroupItem::on_pb_card_clicked()
{
    //发送信号给kernel
    Q_EMIT SIG_clickGroupCard(m_groupId);
}

