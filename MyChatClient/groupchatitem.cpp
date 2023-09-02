#include "groupchatitem.h"
#include "ui_groupchatitem.h"
#include "packDef.h"
#include <QBitmap>
GroupChatItem::GroupChatItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GroupChatItem)
{
    ui->setupUi(this);
}

GroupChatItem::~GroupChatItem()
{
    delete ui;
}
//设置成员属性
void GroupChatItem::setMemberInfo(QString name, int id, int iconId, int status)
{
    //保存属性
    m_id = id;
    m_iconId = iconId;
    m_name = name;

    if(id == 0) {
        name += QString("（我）");
    }
    ui->lb_name->setText(name);

    //设置头像
    //拼接头像文件路径
    QString iconPath = QString(":/tx/%1.png").arg(iconId);

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
    repaint();
}

void GroupChatItem::setOffline()
{
    //设置下线//离线 头像显示灰色
    QString iconPath = QString(":/tx/%1.png").arg(m_iconId);
    QBitmap bmp;
    bmp.load(iconPath);
    ui->pb_icon->setIcon(bmp);
    repaint();
}
