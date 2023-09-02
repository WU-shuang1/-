#include "useritem.h"
#include "ui_useritem.h"
#include "packDef.h"
#include <QBitmap>
UserItem::UserItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UserItem)
{
    ui->setupUi(this);
    m_pFriendCard = new FriendDataCard;
}

UserItem::~UserItem()
{
    delete ui;
    if(m_pFriendCard) {
        delete m_pFriendCard;
        m_pFriendCard = nullptr;
    }
}
//设置卡片状态
void UserItem::setInfo(int id, int icon, int status, QString name, QString feeling)
{
    //保存该卡片的信息
    m_userId = id;
    m_iconId = icon;
    m_status = status;
    m_name = name;
    m_feeling = feeling;

    //设置在卡片上
    ui->lb_name->setText(name);
    ui->lb_feeling->setText(feeling);
    //设置头像
    //拼接头像文件路径
    QString iconPath = QString(":/tx/%1.png").arg(m_iconId);

    //判断用户是否在线
    if(status_online == status){
        //在线 头像亮显
        ui->pb_icon->setIcon(QIcon(iconPath));
    }
    else{
        //离线 头像显示灰色
        QBitmap bmp;
        bmp.load(iconPath);
        ui->pb_icon->setIcon(bmp);
    }
    m_pFriendCard->setFriendInfo(name, feeling, icon, id);
    //立即重绘
    this->repaint();
}
//设置用户为下线状态
void UserItem::setOffline()
{
    //修改用户状态
    m_status = status_offline;
    //离线 头像显示灰色
    QString iconPath = QString(":/tx/%1.png").arg(m_iconId);
    QBitmap bmp;
    bmp.load(iconPath);
    ui->pb_icon->setIcon(bmp);

    //立即重绘
    this->repaint();
}

void UserItem::on_pb_icon_clicked()
{
    //Q_EMIT SIG_clickIcon(m_userId);
    //显示好友资料
    //获取鼠标位置
    QPoint pos = QCursor::pos();
    m_pFriendCard->showNormal();
    int x = pos.x() - m_pFriendCard->width();
    if(x<0) x=0;
    m_pFriendCard->move(QPoint(x,pos.y()) );
}


void UserItem::on_pb_card_clicked()
{
    Q_EMIT SIG_clickCard(m_userId);
}

