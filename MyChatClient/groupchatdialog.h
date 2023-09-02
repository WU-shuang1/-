#ifndef GROUPCHATDIALOG_H
#define GROUPCHATDIALOG_H

#include <QWidget>
#include "groupchatitem.h"
#include <QMap>
#include<QVBoxLayout>
namespace Ui {
class GroupChatDialog;
}

class GroupChatDialog : public QWidget
{
    Q_OBJECT

public:
    explicit GroupChatDialog(QWidget *parent = nullptr);
    ~GroupChatDialog();
public:
    //设置基本信息
    void setInfo(QString groupName, int groupId, int total);
    //设置窗口内容
    void setWindowInfo(QString groupName, int groupId);
    //设置发送来的聊天内容到界面
    void setChatMsg(QString content);

    //添加群成员
    void addMember(GroupChatItem* item, int userId);
    //删除群成员
    void deleteMember(int userId);

    //增加在线人数
    void addOnlineMember();
    //减少在线人数
    void deleteOnlineMember();

    //设置发送来的聊天内容到界面
    void setChatMsg(QString content, int memberId);
signals:
    //把用户输入的内容通过信号发送给Kernel
    void SIG_sendGroupMsg(QString content, int groupId);
    //发送文件信号
    void SIG_sendGroupFile(QString fileName, int groupId);
private slots:
    void on_pb_send_clicked();

    void on_pb_tool1_clicked();

private:
    Ui::GroupChatDialog *ui;
    int m_groupId;
    QString m_groupName;
    int m_totalNum;
    int m_onlineMemberNum;
    QVBoxLayout* m_playout;   //窗口管理器 垂直放置
public:
    QMap<int, GroupChatItem*> m_idToItemMap;    //群成员卡片key为id，value为item
};

#endif // GROUPCHATDIALOG_H
