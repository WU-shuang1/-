#ifndef MYCHATDIALOG_H
#define MYCHATDIALOG_H

#include <QWidget>
#include<QMenu>
#include <QVBoxLayout>
#include<QCloseEvent>
#include "useritem.h"
#include "userdatacard.h"
#include "groupitem.h"
namespace Ui {
class MychatDialog;
}

class MychatDialog : public QWidget
{
    Q_OBJECT

public:
    explicit MychatDialog(QWidget *parent = nullptr);
    ~MychatDialog();


public:
    //重写关闭事件
    void closeEvent(QCloseEvent *event);

    //设置该用户的信息
    void setUserInfo(QString name,QString feeling, int iconId);
    //添加好友
    void addFriend(UserItem* item);
    //删除好友
    void deleteFriend(UserItem* item);

    //添加创建的群聊
    void addMyGroup(GroupItem* item);
    //删除创建的群聊
    void deleteMyGroup(GroupItem* item);
    //添加加入的群聊
    void addJoinGroup(GroupItem* item);
    //删除加入的群聊
    void deleteJoinGroup(GroupItem* item);
signals:
    //发送关闭程序信号给kernel
    void SIG_closeApp();
    //发送添加好友信号给kernel
    void SIG_addFriend(QString tel, QString userName);
    //发送退出登录信号给kernel
    void SIG_offLine();
    //发送保存资料信号给kernel
    void SIG_saveData(QString name ,QString feeling ,int iconId);
    //发送创建群聊的信号给kernel
    void SIG_createGroup(QString name);
    //发送加入群聊的信号给kernel
    void SIG_joinGroup(int groupId);
private slots:
    void on_pb_menu_clicked();
    //处理菜单点击事件
    void slot_triggered(QAction* action);
    //处理群聊点击事件
    void slot_triggered_group(QAction* action);

    void on_pb_tool1_clicked();

    void on_pb_icon_clicked();
    //处理保存资料的槽函数
    void slot_saveUserData(QString name ,QString feeling ,int iconId);

private:
    Ui::MychatDialog *ui;
    UserDataCard* m_pDataCard;
    QVBoxLayout* m_playout;   //窗口管理器 垂直放置
    QVBoxLayout* m_pMyGroupLayout;   //创建的群的窗口管理器
    QVBoxLayout* m_pJoinGroupLayout;    //加入的群的窗口管理器
    QVBoxLayout* m_pMsgLayout;  //消息窗口管理器
    QMenu* m_pMenu;     //菜单
    QMenu* m_pGroupMenu;    //创建、加入群聊菜单
};

#endif // MYCHATDIALOG_H
