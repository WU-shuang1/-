#include "mychatdialog.h"
#include "ui_mychatdialog.h"
#include <QDebug>
#include <QMessageBox>
#include <QInputDialog>
#include <QRegularExpression>
#include "groupchatitem.h"
#include "joindatacard.h"

MychatDialog::MychatDialog(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MychatDialog)
{
    ui->setupUi(this);
    //设置窗口标题
    this->setWindowTitle("MyChat");



    //初始化垂直布局的层
    m_playout = new QVBoxLayout;
    //设置与外控件上下左右的距离
    m_playout->setContentsMargins(0, 0, 0, 0);
    //设置这个层中每个控件彼此之间的距离
    m_playout->setSpacing(3);
    //把层设置到外部的控件上
    ui->wt_list->setLayout(m_playout);

    //初始化创建的群的垂直布局的层
    m_pMyGroupLayout = new QVBoxLayout;
    //设置与外控件上下左右的距离
    m_pMyGroupLayout->setContentsMargins(0, 0, 0, 0);
    //设置这个层中每个控件彼此之间的距离
    m_pMyGroupLayout->setSpacing(3);
    //把层设置到外部的控件上
    ui->wt_createlist->setLayout(m_pMyGroupLayout);

    //初始化加入的群的垂直布局的层
    m_pJoinGroupLayout = new QVBoxLayout;
    //设置与外控件上下左右的距离
    m_pJoinGroupLayout->setContentsMargins(0, 0, 0, 0);
    //设置这个层中每个控件彼此之间的距离
    m_pJoinGroupLayout->setSpacing(3);
    //把层设置到外部的控件上
    ui->wt_joinlist->setLayout(m_pJoinGroupLayout);

    //初始化消息窗口管理器
    m_pMsgLayout = new QVBoxLayout;
    //设置与外控件上下左右的距离
    m_pMsgLayout->setContentsMargins(0, 0, 0, 0);
    //设置这个层中每个控件彼此之间的距离
    m_pMsgLayout->setSpacing(3);
    //把层设置到外部的控件上
    ui->wt_msglist->setLayout(m_pMsgLayout);



    //初始化菜单键 将菜单绑定到菜单键上
    m_pMenu = new QMenu(ui->pb_menu);
    m_pMenu->addAction("退出账号");
    m_pMenu->addAction("添加好友");
    m_pMenu->addAction("系统设置");
    //绑定点击菜单后选择选项的信号和槽
    connect(m_pMenu, SIGNAL(triggered(QAction*)), this, SLOT(slot_triggered(QAction*)));

    m_pGroupMenu = new QMenu(ui->pb_tool1);
    m_pGroupMenu->addAction("创建群聊");
    m_pGroupMenu->addAction("加入群聊");
    //绑定点击群聊后选择选项的信号和槽
    connect(m_pGroupMenu, SIGNAL(triggered(QAction*)), this, SLOT(slot_triggered_group(QAction*)));



    m_pDataCard = new UserDataCard;
    //绑定保存资料的信号和槽
    connect(m_pDataCard, SIGNAL(SIG_saveUserData(QString,QString,int)),
            this, SLOT(slot_saveUserData(QString,QString,int)));

}

MychatDialog::~MychatDialog()
{
    if(ui) {
        delete ui;
        ui = nullptr;
    }
    if(m_playout) {
        delete m_playout;
        m_playout = nullptr;
    }
    if(m_pMenu) {
        delete m_pMenu;
        m_pMenu = nullptr;
    }
    if(m_pGroupMenu) {
        delete m_pGroupMenu;
        m_pGroupMenu = nullptr;
    }
    if(m_pDataCard) {
        delete m_pDataCard;
        m_pDataCard = nullptr;
    }
    if(m_pMyGroupLayout) {
        delete m_pMyGroupLayout;
        m_pMyGroupLayout = nullptr;
    }
    if(m_pJoinGroupLayout) {
        delete m_pJoinGroupLayout;
        m_pJoinGroupLayout = nullptr;
    }
    if(m_pMsgLayout) {
        delete m_pMsgLayout;
        m_pMsgLayout = nullptr;
    }
}
//重写关闭事件
void MychatDialog::closeEvent(QCloseEvent *event)
{
    event->ignore();
    if(QMessageBox::Yes == QMessageBox::question(this, "提示", "是否退出MyChat？")) {
        //回收资源
        if(ui) {
            delete ui;
            ui = nullptr;
        }
        if(m_playout) {
            delete m_playout;
            m_playout = nullptr;
        }
        if(m_pMenu) {
            delete m_pMenu;
            m_pMenu = nullptr;
        }
        if(m_pDataCard) {
            delete m_pDataCard;
            m_pDataCard = nullptr;
        }
        //发送信号给kernel
        Q_EMIT SIG_closeApp();
    }
}
//设置该用户的信息
void MychatDialog::setUserInfo(QString name, QString feeling, int iconId)
{
    //设置昵称和签名
    ui->lb_name->setText(name);
    ui->le_feeling->setText(feeling);
    //设置头像
    QString iconPath = QString(":/tx/%1.png").arg(iconId);
    ui->pb_icon->setIcon(QIcon(iconPath));

    m_pDataCard->SetUserdata(name, feeling, iconId);

    this->repaint();
}
//添加好友
void MychatDialog::addFriend(UserItem *item)
{
    m_playout->addWidget(item);
    QPushButton lb1;
    lb1.resize(QSize(20,60));
    m_playout->addWidget(&lb1);
}
//删除好友
void MychatDialog::deleteFriend(UserItem* item)
{
    m_playout->removeWidget(item);
}
//添加创建的群聊
void MychatDialog::addMyGroup(GroupItem *item)
{
    m_pMyGroupLayout->addWidget(item);
}
//删除创建的群聊
void MychatDialog::deleteMyGroup(GroupItem *item)
{
    m_pMyGroupLayout->removeWidget(item);
}
//添加加入的群聊
void MychatDialog::addJoinGroup(GroupItem *item)
{
    m_pJoinGroupLayout->addWidget(item);
}
//删除加入的群聊
void MychatDialog::deleteJoinGroup(GroupItem *item)
{
    m_pJoinGroupLayout->removeWidget(item);
}

void MychatDialog::on_pb_menu_clicked()
{
    qDebug()<<__func__;
    //获取鼠标位置
    QPoint pos = QCursor::pos();
    //获取菜单的高度
    QSize size = m_pMenu->sizeHint();
    //在鼠标的上方显示
    if(pos.y() - size.height() >=0)
        //在鼠标的上方显示
        m_pMenu->exec(QPoint(pos.x(),pos.y() - size.height()));
    else
        m_pMenu->exec(pos);
}
//处理菜单点击事件
void MychatDialog::slot_triggered(QAction *action)
{
    qDebug()<<__func__;
    if("退出账号" == action->text()) {
        qDebug()<<"退出账号";
        if(QMessageBox::Yes == QMessageBox::question(this, "提示", "是否要退出登录？"))
            Q_EMIT SIG_offLine();
    }
    else if("添加好友" == action->text()) {
        qDebug()<<"添加好友";
        QString strTel = QInputDialog::getText(this, "添加好友","请输入好友的手机号码");
        if(strTel.isEmpty()) return;
        //1.检测手机号是否合法
        QString strTelTemp = strTel;

        if(strTelTemp.remove(" ").isEmpty()) {
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

        //通过信号发送给kernel
        Q_EMIT SIG_addFriend(strTel, ui->lb_name->text());
    }
    else if("系统设置" == action->text()) {
        qDebug()<<"系统设置";
    }
}
//处理群聊点击事件
void MychatDialog::slot_triggered_group(QAction *action)
{
    qDebug()<<__func__;
    if("创建群聊" == action->text()) {
        qDebug()<<"创建群聊";
        QString groupName = QInputDialog::getText(this, "创建群聊","请输入群聊的昵称");
        //1.检测名称是否合法
        if(groupName.isEmpty()) return;
        QString groupNameTemp = groupName;
        if(groupNameTemp.remove(" ").isEmpty()) {
            QMessageBox::about(this, "提示", "群昵称不能为空，请重新输入");
            return;
        }
        //通过信号发送给kernel
        Q_EMIT SIG_createGroup(groupName);
    }
    else if("加入群聊" == action->text()) {
        qDebug()<<"加入群聊";
        QString groupName = QInputDialog::getText(this, "加入群聊","请输入群聊的id");
        //1.检测名称是否合法
        if(groupName.isEmpty()) return;
        QString groupNameTemp = groupName;
        if(groupNameTemp.remove(" ").isEmpty()) {
            QMessageBox::about(this, "提示", "群昵称不能为空，请重新输入");
            return;
        }
        //2.使用正则表达式检测是否都为数字
        QString pattern = "^[0-9]+$";
        QRegularExpression regex(pattern);
        QRegularExpressionMatch match = regex.match(groupName);
        if(!match.hasMatch()){
            QMessageBox::about(this, "提示", "群id不合法必须全为数字，请重新输入");
            return;
        }
        //通过信号发送给kernel
        Q_EMIT SIG_joinGroup(atoi(groupName.toStdString().c_str()));
    }
}


void MychatDialog::on_pb_tool1_clicked()
{
    qDebug()<<__func__;
    //获取鼠标位置
    QPoint pos = QCursor::pos();
    //获取菜单的高度
    QSize size = m_pGroupMenu->sizeHint();
    //在鼠标的上方显示创建群聊和加载群聊的画面
    if(pos.y() - size.height() >=0)
        //在鼠标的上方显示
        m_pGroupMenu->exec(QPoint(pos.x(),pos.y() - size.height()));
    else
        m_pGroupMenu->exec(pos);
}


void MychatDialog::on_pb_icon_clicked()
{
    qDebug()<<__func__;
    //显示资料卡片
    //获取鼠标位置
    QPoint pos = QCursor::pos();
    m_pDataCard->showNormal();
    int x = pos.x() - m_pDataCard->width();
    if(x<0) x=0;
    m_pDataCard->move(QPoint(x,pos.y()) );

}
//处理保存资料的槽函数
void MychatDialog::slot_saveUserData(QString name, QString feeling, int iconId)
{
    qDebug()<<__func__;

    //设置昵称和签名
    ui->lb_name->setText(name);
    ui->le_feeling->setText(feeling);
    //设置头像
    QString iconPath = QString(":/tx/%1.png").arg(iconId);
    ui->pb_icon->setIcon(QIcon(iconPath));
    this->repaint();

    //将资料通过信号发给kernel
    Q_EMIT SIG_saveData(name, feeling, iconId);
}

