#ifndef CKERNEL_H
#define CKERNEL_H

#include <QObject>
#include "logindialog.h"
#include "TcpClientMediator.h"
#include "mychatdialog.h"
#include "packDef.h"
#include "useritem.h"
#include <QMap>
#include "chatdialog.h"
#include "groupitem.h"
#include "groupchatdialog.h"


class CKernel;
typedef void (CKernel::*pfun) (char*, int, long);

class CKernel : public QObject
{
    Q_OBJECT
public:
    explicit CKernel(QObject *parent = nullptr);
signals:

public slots:
    //接收服务端的所有数据
    void slot_dealData(char* buf, int nLen, long lSend);
    //接收登录信号
    void slot_login(QString tel, QString password);
    //接收注册信号
    void slot_register(QString tel, QString password);
    //接收点击好友卡片信号
    void slot_clickCard(int id);
    //把用户输入的内容通过信号发送给Kernel
    void slot_sendMsg(QString content, int id);
    //接收关闭应用的信号
    void slot_closeApp();
    //接收添加好友信号
    void slot_addFriend(QString tel, QString userName);
    //处理退出登录
    void slot_offLine();
    //处理保存资料
    void slot_saveData(QString name ,QString feeling ,int iconId);
    //处理删除好友信号
    void slot_deleteFriend(int id);

    //处理创建群聊的信号
    void slot_createGroup(QString name);
    //处理加入群聊的信号
    void slot_joinGroup(int groupId);
    //处理上传群聊资料的信号
    void slot_updataGroupInfo(QString name, int groupId, int iconId);
    //处理退出群聊的信号
    void slot_exitGroup(int groupId);
    //处理解散群聊的信号
    void slot_deleteGroup(int groupId);
    //处理打开群聊天窗口的信号
    void slot_clickGroupCard(int groupId);
    //处理发送群聊消息
    void slot_sendGroupMsg(QString content, int groupId);

    //处理发送好友文件
    void slot_sendFile(QString FileName, int id);
    //处理发送群聊文件
    void slot_sendGroupFile(QString fileName, int groupId);
    //处理取消发送文件
    void slot_cancel(QString FileId);
    //处理暂停、继续下载/上传文件
    void slot_control(QString FileId,bool flag);
public:
    //处理注册回复
    void dealRegisterRs(char* buf, int nLen, long lSend);
    //处理登录回复
    void dealLoginRs(char* buf, int nLen, long lSend);
    //处理好友信息
    void dealFriendInfo(char* buf, int nLen, long lSend);
    //处理聊天请求
    void dealChatRq(char* buf, int nLen, long lSend);
    //处理下线请求
    void dealOfflineRq(char* buf, int nLen, long lSend);
    //处理添加好友请求
    void dealAddFriendRq(char* buf, int nLen, long lSend);
    //处理添加好友回复
    void dealAddFriendRs(char* buf, int nLen, long lSend);
    //处理删除好友请求
    void dealDeleteFriendRq(char* buf, int nLen, long lSend);
    //处理删除好友回复
    void dealDeleteFriendRs(char* buf, int nLen, long lSend);
    //处理用户数据更新
    void dealUserUpdataRq(char* buf, int nLen, long lSend);

    //处理创建群聊请求回复
    void dealCreateGroupRs(char* buf, int nLen, long lSend);
    //处理群聊信息
    void dealGroupInfo(char* buf, int nLen, long lSend);
    //处理群成员信息
    void dealGroupMemberInfo(char* buf, int nLen, long lSend);
    //处理加入群聊请求回复
    void dealJoinGroupRs(char* buf, int nLen, long lSend);
    //处理群成员下线请求
    void dealGroupMemberOfflineRq(char* buf, int nLen, long lSend);
    //处理解散群聊请求
    void dealGroupDeleteRq(char* buf, int nLen, long lSend);
    //处理退出群聊请求
    void dealGroupExitRq(char* buf, int nLen, long lSend);
    //处理群聊消息请求
    void dealGroupMsg(char* buf, int nLen, long lSend);

    //处理好友发送文件请求
    void dealFriendSendFileRq(char* buf, int nLen, long lSend);
    //处理发送文件回复
    void dealFriendSendFileRs(char* buf, int nLen, long lSend);

    //处理好友发送的数据块
    void dealFileBlockRq(char* buf, int nLen, long lSend);
    //处理文件块回复
    void dealFileBlockRs(char* buf, int nLen, long lSend);
    //处理文件取消发送/接收回复
    void dealFileCancelRs(char* buf, int nLen, long lSend);

    //绑定协议映射表
    void bindProtocolMap();
    //计算MD5哈希值
    QString calculateMD5(const QString& password);
    //UTF-8(QString)转gb2312(char*)，buf是输出参数，传入的是要写入转码后的空间的起始地址，nLen是空间的长度，utf8是UTF-8编码格式的数据
    static void utf8ToGb2312(char* buf, int nLen, QString utf8);
    //gb2312转UTF-8，返回值是是转码后的UTF-8编码格式的数据，参数是gb2312编码格式的字符串
    static QString gb2312ToUtf8(char* buf);

    //加载本地文件
    void loadingLocationFile();
    //回收资源
    void deleteRes();
    //创建指定大小文件
    FILE* CreateLargeFile(char* filePath, unsigned long long fileSize);
public:
    int m_userId;
    QString m_userPath;
    LoginDialog* m_pLogDlg;     //登录窗口
    TcpClientMediator* m_pMediator;
    MychatDialog* m_pMyChat;    //主界面
    pfun m_netProtocolMap[DEF_PROTOCOL_COUNT];			//网络协议映射表
    QMap<int , UserItem*> m_idToItemMap;        //key为用户id，value为好友卡片
    QMap<int , ChatDialog*> m_idToChatDlgMap;        //key为用户id，value为好友聊天窗口
    QMap<int, GroupItem*> m_idToGroupItemMap;   //key为群聊id，value为群聊卡片
    QMap<int, GroupChatDialog*> m_idToGroupDlgMap;  //key-groupid, val-聊天窗口
    QMap<QString, STRU_FILE_INFO*> m_idToFile;   //存储上传/下载文件的信息
};

#endif // CKERNEL_H
