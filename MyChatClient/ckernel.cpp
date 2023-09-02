#include "ckernel.h"
#include "packDef.h"
#include <QMessageBox>
#include <QDebug>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QTextCodec>
#include <QByteArray>
#include "createdatacard.h"
#include "joindatacard.h"
#include "groupchatitem.h"
#include <QTime>
#include <QDir>

#define NetProtocolMap(a) (m_netProtocolMap[a - _DEF_PROTOCOL_BASE - 1])

CKernel::CKernel(QObject *parent) : QObject(parent)
{
    //创建该应用本地文件夹
    QString path = "D:/Client";

    if(!QDir().mkpath(path)) {
        qDebug()<<__func__<< " mkdir error";
        return;
    }
    //绑定网络协议映射表
    bindProtocolMap();
    //创建登录窗口
    m_pLogDlg = new LoginDialog;
    //绑定注册按键的处理函数
    connect(m_pLogDlg, SIGNAL(SIG_register(QString,QString)),
            this, SLOT(slot_register(QString,QString)));
    //绑定登录按键的处理函数
    connect(m_pLogDlg, SIGNAL(SIG_login(QString,QString)),
            this, SLOT(slot_login(QString,QString)));
    m_pLogDlg->showNormal();


    //初始化网络
    m_pMediator = nullptr;
    m_pMediator = new TcpClientMediator;
    //绑定接受数据的信号和槽
   connect(m_pMediator, SIGNAL(SIG_dealData(char*,int,long)),
            this, SLOT(slot_dealData(char*,int,long)));
    //打开网络 不在此处打开网络 避免打开软件时卡顿 后续将它优化在点击登录或注册时再打开网络
    //同时这样可以灵活打开关闭网络就可以实现不同的在线状态
    if(!m_pMediator->OpenNet()) {
        QMessageBox::about(m_pLogDlg, "提示", "网络连接失败");
        return;
    }
    //测试发送数据
    //m_pMediator->SendData("hello world", sizeof("hello world"),0);
}
//接收服务端的所有数据
void CKernel::slot_dealData(char *buf, int nLen, long lSend)
{
    //qDebug()<<__func__;
    //判断包类型
    PackType type = *(PackType*)buf;

    if (_DEF_PROTOCOL_BASE < type && type <= _DEF_PROTOCOL_BASE + DEF_PROTOCOL_COUNT) {
            pfun fun = NetProtocolMap(type);
            (this->*fun)(buf, nLen, lSend);
    }
    delete[] buf;
    buf = nullptr;
}
//处理登录
void CKernel::slot_login(QString tel, QString password)
{
    qDebug()<<__func__;
    //打包数据
    STRU_TCP_LOGIN_RQ rq;
    strcpy(rq.tel, tel.toStdString().c_str());

    //对密码+电话号码进行计算MD5哈希值 + tel
    QString strMD5 = calculateMD5(password);
    strcpy(rq.password, strMD5.toStdString().c_str());
    qDebug() << rq.password;
    //将登录信息通过中介者发送给服务端
    if(!m_pMediator->SendData((char*)&rq, sizeof(rq), 0)){
        qDebug()<<__func__ << "SendData error";
    }
}
//处理注册
void CKernel::slot_register(QString tel, QString password)
{
    qDebug()<<__func__;
    //打包数据
    STRU_TCP_REGISTER_RQ rq;
    qDebug()<< tel<<"  "<<password;
    strcpy(rq.tel, tel.toStdString().c_str());

    //对密码进行计算MD5哈希值
    QString strMD5 = calculateMD5(password);
    utf8ToGb2312(rq.password, DEF_PASSWORD_SIZE, strMD5);

    //创建该用户的本地文件夹用来存储一些后续的功能所需的数据

    //将注册信息通过中介者发送给服务端
    m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
}
//接收点击好友卡片信号
void CKernel::slot_clickCard(int id)
{
    if(m_idToChatDlgMap.find(id) == m_idToChatDlgMap.end()) {
        return;
    }
    //显示窗口
    m_idToChatDlgMap[id]->showNormal();
}
//把用户输入的内容通过信号发送给Kernel
void CKernel::slot_sendMsg(QString content, int id){
    if(m_idToItemMap.find(id) == m_idToItemMap.end()) return;
    UserItem* item = m_idToItemMap[id];
    //检测用户是否在线
    if(item->m_status == status_offline) {
        //不在线 对话框中提示该用户好友不在线
        m_idToChatDlgMap[id]->setFriendOffline();
    }

    //打包
    STRU_TCP_CHAT_RQ rq;
    //设置包内容
    rq.userId = m_userId;
    rq.friendId = id;
    strcpy(rq.content, content.toStdString().c_str());

    //发送给服务端 离线状态发送给客户端为了后续的版本更新隐身状态或上线后将消息发送给好友
    m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
}
//接收关闭应用的信号
void CKernel::slot_closeApp()
{
    qDebug()<<__func__;
    //发送下线请求给服务端
    STRU_TCP_OFFLINE_RQ rq;
    rq.userId = m_userId;

    m_pMediator->SendData((char*)&rq, sizeof(rq), 0);

    //回收资源
    deleteRes();

    //回收登录
    if(m_pLogDlg) {
        m_pLogDlg->hide();
        delete m_pLogDlg;
        m_pLogDlg = nullptr;
    }

    //退出应用
    exit(0);
}
//接收添加好友信号
void CKernel::slot_addFriend(QString tel, QString userName)
{
    qDebug()<<__func__;
    //打包
    STRU_TCP_ADDFRIEND_RQ rq;
    rq.userId = m_userId;
    strcpy(rq.userName, userName.toStdString().c_str());
    strcpy(rq.friendTel, tel.toStdString().c_str());

    //发送给服务端
    m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
}
//处理退出登录
void CKernel::slot_offLine()
{
    //发送下线请求给服务端
    STRU_TCP_OFFLINE_RQ rq;
    rq.userId = m_userId;

    m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
    //回收资源
    deleteRes();
    //显示登录界面
    m_pLogDlg->showNormal();

    m_pMediator = new TcpClientMediator;
    //绑定接受数据的信号和槽
   connect(m_pMediator, SIGNAL(SIG_dealData(char*,int,long)),
            this, SLOT(slot_dealData(char*,int,long)));
    //打开网络 不在此处打开网络 避免打开软件时卡顿 后续将它优化在点击登录或注册时再打开网络
    //同时这样可以灵活打开关闭网络就可以实现不同的在线状态
    if(!m_pMediator->OpenNet()) {
        QMessageBox::about(m_pLogDlg, "提示", "网络连接失败");
        return;
    }
}
//处理保存资料
void CKernel::slot_saveData(QString name, QString feeling, int iconId)
{
    qDebug()<<__func__;
    //将资料打包发送给服务端
    STRU_TCP_UPDATA_RQ rq;
    rq.iconId = iconId;
    rq.userId = m_userId;
    utf8ToGb2312(rq.userName, DEF_NAME_SIZE, name);
    utf8ToGb2312(rq.userFeeling, DEF_FEELING_SIZE, feeling);

    m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
}
//处理删除好友信号
void CKernel::slot_deleteFriend(int id)
{
    qDebug()<<__func__;
    //打包发送给服务端
    STRU_TCP_DELETEFRIEND_RQ rq;
    rq.friendId = id;
    rq.userId = m_userId;

    m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
}
//处理创建群聊的信号
void CKernel::slot_createGroup(QString name)
{
    qDebug()<<__func__;
    //打包发送给服务端
    STRU_TCP_CREATEGROUP_RQ rq;
    rq.userId = m_userId;
    utf8ToGb2312(rq.groupName, DEF_NAME_SIZE, name);

    m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
}
//处理加入群聊的信号
void CKernel::slot_joinGroup(int groupId)
{
    qDebug()<<__func__;
    //打包发送给服务端
    STRU_TCP_JOINGROUP_RQ rq;
    rq.userId = m_userId;
    rq.groupId = groupId;

    m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
}
//处理上传群聊资料的信号
void CKernel::slot_updataGroupInfo(QString name, int groupId, int iconId)
{
    qDebug()<<__func__;
    //打包发送给服务端
    STRU_TCP_GROUOP_INFO info;
    info.userId = m_userId;
    info.groupId = groupId;
    info.groupIconId = iconId;
    utf8ToGb2312(info.name, DEF_NAME_SIZE, name);

    m_pMediator->SendData((char*)&info, sizeof(info), 0);
}
//处理退出群聊的信号
void CKernel::slot_exitGroup(int groupId)
{
    qDebug()<<__func__;
    //回收窗口和群聊卡片
    //删除item
    auto ite = m_idToGroupItemMap.find(groupId);
    if(ite != m_idToGroupItemMap.end()) {
        //从加入的群中移除群聊卡片
        m_pMyChat->deleteJoinGroup(*ite);
        delete (*ite);
        (*ite) = nullptr;
        m_idToGroupItemMap.erase(ite);
    }
    //删除聊天窗口
    auto iteDlg = m_idToGroupDlgMap.find(groupId);
    if(iteDlg != m_idToGroupDlgMap.end()) {
        (*iteDlg)->hide();
        delete (*iteDlg);
        (*iteDlg) = nullptr;
        m_idToGroupDlgMap.erase(iteDlg);
    }
    //打包发送给服务端
    STRU_TCP_GROUP_DELETE_RQ rq;
    rq.type = _DEF_TCP_MEMBER_EXIT_RQ;
    rq.groupId = groupId;
    rq.userId = m_userId;

    m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
}
//处理解散群聊的信号
void CKernel::slot_deleteGroup(int groupId)
{
    qDebug()<<__func__;
    //打包发送给服务端
    STRU_TCP_GROUP_DELETE_RQ rq;
    rq.groupId = groupId;
    rq.userId = m_userId;

    m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
}
//处理打开群聊天窗口的信号
void CKernel::slot_clickGroupCard(int groupId)
{
    qDebug()<<__func__;
    //打开对应群的聊天窗口
    if(m_idToGroupDlgMap.find(groupId) != m_idToGroupDlgMap.end()) {
        m_idToGroupDlgMap[groupId]->showNormal();
    }
}
//处理发送群聊消息
void CKernel::slot_sendGroupMsg(QString content, int groupId)
{
    qDebug()<<__func__;
    //打包发送至服务端
    STRU_TCP_GROUP_CHAT_RQ rq;
    rq.groupId = groupId;
    rq.userId = m_userId;
    strcpy(rq.content, content.toStdString().c_str());

    m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
}
//处理发送好友文件
void CKernel::slot_sendFile(QString FileName, int id)
{
    qDebug()<<__func__;
    //保存文件信息
    STRU_FILE_INFO* info = new STRU_FILE_INFO;

    char name[DEF_FILE_NAME_SIZE] = "";
    utf8ToGb2312(name, DEF_FILE_NAME_SIZE, FileName);
    FILE* pFile = fopen(name,"rb");
    if(pFile == nullptr) {
        QMessageBox::about(m_pMyChat,"提示", "文件打开失败");
        return;
    }
    info->pFile = pFile;
    if(_fseeki64(pFile, 0, SEEK_END) != 0) {
        QMessageBox::about(m_pMyChat,"提示", "文件偏移失败");
        return;
    }
    info->fileSize = _ftelli64(pFile);
    _fseeki64(pFile, 0, SEEK_SET);

    strcpy(info->filePATH, FileName.toStdString().c_str());
    int i;
    for(i = FileName.size()-1;i>=0;i--) {
        if(FileName[i] == '/' || FileName[i] == '\\') {
            break;
        }
    }
    strcpy(info->fileName, FileName.toStdString().substr(i+1).c_str());

    QString fileId = QString("%1_%2").arg(info->fileName).arg(QTime::currentTime().toString("hh_mm_ss_zzz"));
    strcpy(info->fileId, fileId.toStdString().c_str());


    info->filePos = 0;
    info->userId = m_userId;
    info->friendId = id;
    m_idToFile[info->fileId] = info;


    //打包发送至服务端
    STRU_TCP_SEND_FILE_RQ rq;
    utf8ToGb2312(rq.fileId, DEF_FILE_NAME_SIZE, info->fileId);
    utf8ToGb2312(rq.fileName,DEF_FILE_NAME_SIZE, info->fileName);
    rq.fileSize = info->fileSize;
    rq.userId = m_userId;
    rq.friendId = id;

    m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
}
//处理发送群聊文件
void CKernel::slot_sendGroupFile(QString fileName, int groupId)
{

}
//处理取消发送文件
void CKernel::slot_cancel(QString FileId)
{
    qDebug()<<__func__;
    //打包发送给服务端
    STRU_TCP_FILE_CANCEL_RQ rq;
    utf8ToGb2312( rq.fileId, DEF_FILE_ID_SIZE, FileId);

    //再发送取消发送
     m_pMediator->SendData((char*)&rq, sizeof(rq), 0);


}
//处理暂停、继续下载/上传文件
void CKernel::slot_control(QString FileId, bool flag)
{
    qDebug()<<__func__;
    //打包发送给服务端
    STRU_TCP_FILE_PAUSE_RQ rq;
    utf8ToGb2312( rq.fileId, DEF_FILE_ID_SIZE, FileId);
    rq.flag = flag;
    if(m_idToFile.find(FileId) == m_idToFile.end()) return;
    m_idToFile[FileId]->status = flag;
    rq.kind = m_idToFile[FileId]->kind;

     m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
}

//处理注册回复
void CKernel::dealRegisterRs(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_REGISTER_RS rs = *(STRU_TCP_REGISTER_RS*)buf;
    //查看注册结果
    if(rs.result == register_success) {
        //注册成功
        QMessageBox::about(m_pLogDlg, "提示", "注册成功");
    }else if(rs.result == tel_is_exist){
        //注册失败
        QMessageBox::about(m_pLogDlg, "提示", "该用户已存在");
    }
}

void CKernel::dealLoginRs(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_LOGIN_RS* rs = (STRU_TCP_LOGIN_RS*)buf;

    //查看注册结果
    switch (rs->result) {
    case login_success:
    {
        //登陆成功 保存id
        m_userId = rs->userId;

        //加载本地用户缓存文件
        loadingLocationFile();
        //创建主界面
        m_pMyChat = new MychatDialog;
        //绑定关闭应用的信号和槽
        connect(m_pMyChat, SIGNAL(SIG_closeApp()), this, SLOT(slot_closeApp()));
        //绑定添加好友的信号和槽
        connect(m_pMyChat, SIGNAL(SIG_addFriend(QString,QString)), this, SLOT(slot_addFriend(QString,QString)));
        //绑定退出登录的信号和槽
        connect(m_pMyChat, SIGNAL(SIG_offLine()), this ,SLOT(slot_offLine()));
        //绑定上传资料的信号和槽函数
        connect(m_pMyChat, SIGNAL(SIG_saveData(QString,QString,int)),
                this, SLOT(slot_saveData(QString,QString,int)));
        //绑定创建群聊的信号和槽函数
        connect(m_pMyChat, SIGNAL(SIG_createGroup(QString)), this, SLOT(slot_createGroup(QString)));
        //绑定加入群聊的信号和槽函数
        connect(m_pMyChat, SIGNAL(SIG_joinGroup(int)), this, SLOT(slot_joinGroup(int)));

        //隐藏登录窗口 打开主窗口
        m_pLogDlg->hide();
        m_pMyChat->showNormal();

    }
        break;
    case password_error:
    {
        //密码错误
        QMessageBox::about(m_pLogDlg, "提示", "密码错误，请重新输入");
    }
        break;
    case tel_error:
    {
        //该用户不存在
        QMessageBox::about(m_pLogDlg, "提示", "该用户不存在");
    }
        break;
    case had_login:
    {
        //已经登录过
        QMessageBox::about(m_pLogDlg, "提示", "该用户在其他位置登录");
    }
        break;
    }
}
//处理好友信息
void CKernel::dealFriendInfo(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_FRIEND_INFO* info = (STRU_TCP_FRIEND_INFO*)buf;

    //判断是否为自己的信息
    if(info->friendId == m_userId) {
        m_pMyChat->setUserInfo(gb2312ToUtf8(info->name), gb2312ToUtf8(info->feeling), info->iconId);
        return;
    }

    //判断好友卡片是否存在
    if(m_idToItemMap.find(info->friendId) == m_idToItemMap.end()) {
        //qDebug()<<info->friendId;
        //创建好友卡片
        UserItem* item = new UserItem;
        //绑定点击头像的信号和槽
        connect(item, SIGNAL(SIG_clickCard(int)), this, SLOT(slot_clickCard(int)));
        m_pMyChat->addFriend(item);
        //绑定删除好友的信号和槽
        connect(item->m_pFriendCard, SIGNAL(SIG_deleteFriend(int)), this, SLOT(slot_deleteFriend(int)));

        //将Item保存在map中
        m_idToItemMap[info->friendId] = item;

        //创建好友聊天窗口
        ChatDialog* chatdlg = new ChatDialog;
        //绑定发送聊天内容的信号和槽
        connect(chatdlg, SIGNAL(SIG_sendMsg(QString,int)), this, SLOT(slot_sendMsg(QString,int)));
        //绑定发送文件的信号和槽
        connect(chatdlg, SIGNAL(SIG_sendFile(QString,int)), this, SLOT(slot_sendFile(QString,int)));
        //绑定暂停、继续发送/下载文件的信号和槽
        connect(chatdlg, SIGNAL(SIG_control(QString,bool)), this, SLOT(slot_control(QString,bool)));
        //绑定取消发送/下载文件的信号和槽
        connect(chatdlg, SIGNAL(SIG_cancel(QString)), this, SLOT(slot_cancel(QString)));
        chatdlg->setWindowInfo(gb2312ToUtf8(info->name), info->friendId);
        //将窗口隐藏
        chatdlg->hide();
        //将窗口保存在map中
        m_idToChatDlgMap[info->friendId] = chatdlg;

        //判断是否有该好友未发送/接收完的文件
        auto ite = m_idToFile.begin();
        while(ite != m_idToFile.end()) {
            if( (*ite)->friendId == info->friendId || (*ite)->userId == info->friendId) {
                //发送文件上传请求
                if((*ite)->kind == filekind_send) {
                    chatdlg->showNormal();
                    STRU_TCP_SEND_FILE_RQ rq;
                    utf8ToGb2312(rq.fileId, DEF_FILE_NAME_SIZE, (*ite)->fileId);
                    utf8ToGb2312(rq.fileName,DEF_FILE_NAME_SIZE, (*ite)->fileName);
                    rq.fileSize = (*ite)->fileSize;
                    rq.filePos = (*ite)->filePos;
                    rq.userId = m_userId;
                    rq.friendId = (*ite)->friendId;

                    m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
                }
            }
            ite++;
        }
    }

    //更新好友信息
    m_idToItemMap[info->friendId]->setInfo(info->friendId, info->iconId, info->status,
                                           gb2312ToUtf8(info->name), gb2312ToUtf8(info->feeling));

}
//处理聊天请求
void CKernel::dealChatRq(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_CHAT_RQ* rq = (STRU_TCP_CHAT_RQ*)buf;
    //查看是否存在与该好友的聊天窗口
    if(m_idToChatDlgMap.find(rq->friendId) == m_idToChatDlgMap.end()) return;
    //将内容设置到聊天窗口 并显示
    m_idToChatDlgMap[rq->friendId]->setChatMsg(rq->content);
    m_idToChatDlgMap[rq->friendId]->showNormal();
}
//处理下线请求
void CKernel::dealOfflineRq(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_OFFLINE_RQ* rq = (STRU_TCP_OFFLINE_RQ*)buf;
    if(m_idToItemMap.find(rq->userId) == m_idToItemMap.end()) return;
    //将好友状态设置为下线状态
    m_idToItemMap[rq->userId]->setOffline();
}
//处理添加好友请求
void CKernel::dealAddFriendRq(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_ADDFRIEND_RQ* rq = (STRU_TCP_ADDFRIEND_RQ*)buf;
    STRU_TCP_ADDFRIEND_RS rs;
    if(QMessageBox::Yes == QMessageBox::question(m_pMyChat, "提示",
                                                 QString("用户【%1】请求添加你为好友，是否同意？").arg(rq->userName))) {
        //同意
        rs.result = addfriend_success;
    }else {
        //拒绝
        rs.result = addfriend_refuse;
    }
    rs.userId = m_userId;
    rs.friendId = rq->userId;

    //打包发送给服务端
    m_pMediator->SendData((char*)&rs, sizeof(rs), 0);
}
//处理添加好友回复
void CKernel::dealAddFriendRs(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_ADDFRIEND_RS* rs = (STRU_TCP_ADDFRIEND_RS*)buf;
    //判断是否添加成功
    switch (rs->result) {
        case addfriend_success:
        {
            //添加成功
            QMessageBox::about(m_pMyChat, "提示", QString("【%1】同意了你的好友请求").arg(gb2312ToUtf8(rs->userName)));
        }
        break;
        case addfriend_not_exist:
        {
            //用户不存在
            QMessageBox::about(m_pMyChat, "提示", "该用户不存在");
        }
        break;
        case addfriend_refuse:
        {
            //被拒绝
            QMessageBox::about(m_pMyChat, "提示", QString("【%1】拒绝了你的好友请求").arg(gb2312ToUtf8(rs->userName)));
        }
        break;
        case addfriend_offline:
        {
            //用户不在线
            QMessageBox::about(m_pMyChat, "提示", "该用户不在线");
        }
        break;
    case addfriend_mine:
    {
        //不能添加自己为好友
        QMessageBox::about(m_pMyChat, "提示", "不能添加自己为好友");
    }
    break;
    }
}
//处理删除好友请求
void CKernel::dealDeleteFriendRq(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_DELETEFRIEND_RQ* rq = (STRU_TCP_DELETEFRIEND_RQ*)buf;

    //将好友的卡片删除
    if(m_idToItemMap.find(rq->userId) != m_idToItemMap.end()) {
        delete m_idToItemMap[rq->userId];
        m_idToItemMap[rq->userId] = nullptr;
        m_idToItemMap.erase(m_idToItemMap.find(rq->userId));
    }
}
//处理删除好友回复
void CKernel::dealDeleteFriendRs(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_DELETEFRIEND_RS* rs = (STRU_TCP_DELETEFRIEND_RS*)buf;
    if(rs->result == deletefriend_failed) {
        QMessageBox::about(m_pMyChat, "提示", "删除好友失败");
        return;
    }
    //将好友的卡片删除
    m_pMyChat->deleteFriend(m_idToItemMap[rs->friendId]);
    delete m_idToItemMap[rs->friendId];
    m_idToItemMap[rs->friendId] = nullptr;
    m_idToItemMap.erase(m_idToItemMap.find(rs->friendId));

    QMessageBox::about(m_pMyChat, "提示", "删除好友成功");
}
//处理用户数据更新
void CKernel::dealUserUpdataRq(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_UPDATA_RQ* rq = (STRU_TCP_UPDATA_RQ*)buf;

    QString name = gb2312ToUtf8(rq->userName);
    QString feeling = gb2312ToUtf8(rq->userFeeling);

    if(m_idToItemMap.find(rq->userId) != m_idToItemMap.end()) {
        //更新该好友的信息
        m_idToItemMap[rq->userId]->setInfo(rq->userId, rq->iconId, status_online, name, feeling);
    }
}
//处理创建群聊请求回复
void CKernel::dealCreateGroupRs(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_CREATEGROUP_RS* rs = (STRU_TCP_CREATEGROUP_RS*)buf;
    if(rs->result == creategroup_success) {
        QMessageBox::about(m_pMyChat, "提示", "创建群聊成功");
    }else {
        QMessageBox::about(m_pMyChat, "提示", "创建群聊失败");
    }
}
//处理群聊信息
void CKernel::dealGroupInfo(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_GROUOP_INFO* info = (STRU_TCP_GROUOP_INFO*)buf;
    //判断是否存在该群聊卡片
    if(m_idToGroupItemMap.find(info->groupId) == m_idToGroupItemMap.end()) {
        //创建群聊卡片
        GroupItem* item = new GroupItem;

        //绑定点击群聊卡片的信号和槽
        connect(item, SIGNAL(SIG_clickGroupCard(int)), this, SLOT(slot_clickGroupCard(int)));

        //判断群主id是否为用户id
        if(m_userId == info->userId) {
            //绑定对应的信号和槽
            item->m_pDataCard = new CreateDataCard;
            CreateDataCard* pcard = (CreateDataCard*)item->m_pDataCard;
            //更新群聊信息的信号和槽
            connect(pcard, SIGNAL(SIG_updataGroupInfo(QString,int,int)),
                    this, SLOT(slot_updataGroupInfo(QString,int,int)));
            //解散群聊的信号和槽
            connect(pcard, SIGNAL(SIG_deleteGroup(int)), this, SLOT(slot_deleteGroup(int)));
            //将它加入 创建的群窗口中
            m_pMyChat->addMyGroup(item);
        }else {
            //绑定对应的信号和槽
            item->m_pDataCard = new JoinDataCard;
            JoinDataCard* pcard = (JoinDataCard*)item->m_pDataCard;
            connect(pcard, SIGNAL(SIG_exitGroup(int)), this, SLOT(slot_exitGroup(int)));

            //将它加入 加入的群窗口中
            m_pMyChat->addJoinGroup(item);
        }

        //创建聊天窗口
        GroupChatDialog* chatDlg = new GroupChatDialog;
        //绑定相应的信号和槽
        //绑定发送信息
        connect(chatDlg, SIGNAL(SIG_sendGroupMsg(QString,int)), this, SLOT(slot_sendGroupMsg(QString,int)));
        //绑定发送文件
        connect(chatDlg, SIGNAL(SIG_sendGroupFile(QString,int)), this, SLOT(slot_sendFile(QString,int)));

        chatDlg->hide();
        //将Dlg添加值map中
        m_idToGroupDlgMap[info->groupId] = chatDlg;
        //将item添加进map中
        m_idToGroupItemMap[info->groupId] = item;
    }

    //设置群聊天窗口属性
    m_idToGroupDlgMap[info->groupId]->setInfo(gb2312ToUtf8(info->name), info->groupId, info->total);
    //设置群聊卡片属性
    m_idToGroupItemMap[info->groupId]->setGroupInfo(gb2312ToUtf8(info->name), info->groupIconId,
                                                    info->total, info->groupId);
}
//处理群成员信息
void CKernel::dealGroupMemberInfo(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_GROUOP_MEMBER_INFO* info = (STRU_TCP_GROUOP_MEMBER_INFO*)buf;

    if(info->userId == m_userId) {
        //是自己的信息
        if(m_idToGroupDlgMap.find(info->groupId) != m_idToGroupDlgMap.end()) {
            //添加成员
            GroupChatItem* item = nullptr;
            if(m_idToGroupDlgMap[info->groupId]->m_idToItemMap.find(info->userId)
                    == m_idToGroupDlgMap[info->groupId]->m_idToItemMap.end()) {
                //没有群成员卡片 添加卡片
                item = new GroupChatItem;
                m_idToGroupDlgMap[info->groupId]->addMember(item, info->userId);
            }
            if(info->status == status_online) {
                //在线状态
                m_idToGroupDlgMap[info->groupId]->addOnlineMember();
            }
            //传入id为0
            m_idToGroupDlgMap[info->groupId]->m_idToItemMap[info->userId]->setMemberInfo(gb2312ToUtf8(info->userName), 0, info->userIconId, info->status);
        }
        return;
    }

    if(m_idToGroupDlgMap.find(info->groupId) != m_idToGroupDlgMap.end()) {
        GroupChatItem* item = nullptr;
        //添加成员
        if(m_idToGroupDlgMap[info->groupId]->m_idToItemMap.find(info->userId)
                == m_idToGroupDlgMap[info->groupId]->m_idToItemMap.end()) {
            //没有群成员卡片 添加卡片
            item = new GroupChatItem;
            m_idToGroupDlgMap[info->groupId]->addMember(item, info->userId);
        }
        if(info->status == status_online) {
            //在线状态
            m_idToGroupDlgMap[info->groupId]->addOnlineMember();
        }
        m_idToGroupDlgMap[info->groupId]->m_idToItemMap[info->userId]->setMemberInfo(gb2312ToUtf8(info->userName), info->userId, info->userIconId, info->status);
    }
}
//处理加入群聊请求回复
void CKernel::dealJoinGroupRs(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_JOINGROUP_RS* rs = (STRU_TCP_JOINGROUP_RS*)buf;
    if(rs->result == joingroup_success) {
        QMessageBox::about(m_pMyChat, "提示", "添加成功");
    }
    else if(rs->result == joingroup_not_exist) {
        QMessageBox::about(m_pMyChat, "提示", "添加失败，群聊id不存在");
    }
}
//处理群成员下线请求
void CKernel::dealGroupMemberOfflineRq(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_GROUP_MEMBER_OFFLINE_RQ* rq = (STRU_TCP_GROUP_MEMBER_OFFLINE_RQ*)buf;

    //找到群
    if(m_idToGroupDlgMap.find(rq->groupId) != m_idToGroupDlgMap.end()) {
        if(m_idToGroupDlgMap[rq->groupId]->m_idToItemMap.find(rq->userId)
                != m_idToGroupDlgMap[rq->groupId]->m_idToItemMap.end()){
            //减少一个在线人数
            m_idToGroupDlgMap[rq->groupId]->deleteOnlineMember();
            //找到该用户卡片 设置为下线
             m_idToGroupDlgMap[rq->groupId]->m_idToItemMap[rq->userId]->setOffline();
        }
    }
}
//处理解散群聊请求
void CKernel::dealGroupDeleteRq(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_GROUP_DELETE_RQ* rq = (STRU_TCP_GROUP_DELETE_RQ*)buf;
    //删除item
    auto ite = m_idToGroupItemMap.find(rq->groupId);
    if(ite != m_idToGroupItemMap.end()) {
        if(rq->userId == m_userId) {
            //解散自己的群 从创建的群中移除群聊卡片
            m_pMyChat->deleteMyGroup(*ite);
        }
        else {
            //从加入的群中移除群聊卡片
            m_pMyChat->deleteJoinGroup(*ite);
        }
        delete (*ite);
        (*ite) = nullptr;
        m_idToGroupItemMap.erase(ite);
    }
    //删除聊天窗口
    auto iteDlg = m_idToGroupDlgMap.find(rq->groupId);
    if(iteDlg != m_idToGroupDlgMap.end()) {
        (*iteDlg)->hide();
        delete (*iteDlg);
        (*iteDlg) = nullptr;
        m_idToGroupDlgMap.erase(iteDlg);
    }
}
//处理退出群聊请求
void CKernel::dealGroupExitRq(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_GROUP_DELETE_RQ* rq = (STRU_TCP_GROUP_DELETE_RQ*)buf;

    //锁定该群
    if(m_idToGroupDlgMap.find(rq->groupId) == m_idToGroupDlgMap.end()) return;
    m_idToGroupDlgMap[rq->groupId]->deleteMember(rq->userId);
}
//处理群聊消息请求
void CKernel::dealGroupMsg(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_GROUP_CHAT_RQ* rq = (STRU_TCP_GROUP_CHAT_RQ*)buf;
    if(m_idToGroupDlgMap.find(rq->groupId) == m_idToGroupDlgMap.end()) return;

    m_idToGroupDlgMap[rq->groupId]->setChatMsg(rq->content, rq->userId);
    m_idToGroupDlgMap[rq->groupId]->showNormal();
}
//处理好友发送文件请求
void CKernel::dealFriendSendFileRq(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_SEND_FILE_RQ* rq = (STRU_TCP_SEND_FILE_RQ*)buf;
    QString fileId = gb2312ToUtf8(rq->fileId);
    //准备发送回复
    STRU_TCP_SEND_FILE_RS rs;

    //检测是否是未传送完的文件
    if(m_idToFile.find(fileId) == m_idToFile.end()) {
        //第一次传输、创建文件
        QString fileName = m_userPath + QString("/%1").arg(gb2312ToUtf8(rq->fileName));
        FILE* pFile = CreateLargeFile((char*)fileName.toStdString().c_str(), rq->fileSize);
        if (pFile == nullptr) {
                cout << "CreateLargeFile error" << GetLastError() << endl;
                rs.result = sendfile_failed;
                m_pMediator->SendData((char*)&rs, sizeof(rs), 0);
                return;
        }

        //保存文件信息
        STRU_FILE_INFO* info = new STRU_FILE_INFO;
        info->filePos = 0;
        info->pFile = pFile;
        strcpy(info->fileId, fileId.toStdString().c_str());
        strcpy(info->fileName, gb2312ToUtf8(rq->fileName).toStdString().c_str());
        strcpy(info->filePATH, fileName.toStdString().c_str());

        info->fileSize = rq->fileSize;
        info->userId = rq->userId;
        info->friendId = rq->friendId;

        //接收文件、初始为暂停接收
        info->kind = filekind_recv;
        info->status = sendfile_pause;
        m_idToFile[fileId] = info;
    }

    //发送文件请求回复
    strcpy(rs.fileId, rq->fileId);
    rs.result = sendfile_success;

    m_pMediator->SendData((char*)&rs, sizeof(rs), 0);

    //将好友窗口显示，并添加进度条
    //添加任务
    STRU_FILE_INFO* info = m_idToFile[fileId];
    ChatDialog* pChatDlg = m_idToChatDlgMap[rq->userId];
    pChatDlg->addProcessBar(info->fileName,info->fileId, info->kind);
    pChatDlg->showNormal();
}
//处理发送文件回复
void CKernel::dealFriendSendFileRs(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_SEND_FILE_RS* rs = (STRU_TCP_SEND_FILE_RS*)buf;
    QString fileId = gb2312ToUtf8(rs->fileId);
    STRU_FILE_INFO* pFileInfo = m_idToFile[fileId];
    ChatDialog* pChatDlg = m_idToChatDlgMap[rs->friendId];
    if(!pFileInfo || !pChatDlg) {
        qDebug()<<"读取文件信息或用户窗口出现错误";
        //return;
    }

    if(rs->result == sendfile_failed) {
        QMessageBox::about(pChatDlg, "提示","文件发送失败");
        return;
    }
    //添加任务
    pChatDlg->addProcessBar(pFileInfo->fileName,pFileInfo->fileId, pFileInfo->kind);

    //读取文件进行发送
    STRU_TCP_FILE_BLOCK_RQ rq;
    rq.filePos = pFileInfo->filePos;
    _fseeki64(pFileInfo->pFile, pFileInfo->filePos, SEEK_SET);
    rq.blockSize = fread(rq.block,sizeof(char), DEF_FILE_BLOCK_SIZE, pFileInfo->pFile);
    utf8ToGb2312(rq.fileId, DEF_FILE_NAME_SIZE, fileId);

    m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
}
//处理好友发送的数据块
void CKernel::dealFileBlockRq(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_FILE_BLOCK_RQ* rq = (STRU_TCP_FILE_BLOCK_RQ*)buf;

    QString fileId = gb2312ToUtf8(rq->fileId);
    STRU_FILE_INFO* pFileInfo = m_idToFile[fileId];
    if(pFileInfo->status == sendfile_pause) return;

    ChatDialog* pChatDlg = m_idToChatDlgMap[pFileInfo->userId];

    //写入文件中
    unsigned long long pos = 0;
    if(rq->filePos == 0 && pFileInfo->filePos > 0 ) {
        //断点续传
        pos = pFileInfo->filePos;
    }else {
        pos = min(rq->filePos, pFileInfo->filePos);
    }
    _fseeki64(pFileInfo->pFile, pos, SEEK_SET);
    long long  writeNum = fwrite(rq->block, sizeof(char), rq->blockSize, pFileInfo->pFile);
    pFileInfo->filePos = pos + writeNum;


    //修改进度条占比
    int value = (long double)pFileInfo->filePos / (pFileInfo->fileSize) * 100;
    pChatDlg->updataProcessBar(fileId ,value);

    //向服务端发送当前已经保存的数据
    STRU_TCP_FILE_BLOCK_RS rs;
    strcpy(rs.fileId, rq->fileId);

    rs.filePos = pFileInfo->filePos;
    rs.friendId = pFileInfo->friendId;

    m_pMediator->SendData((char*)&rs, sizeof(rs), 0);

    //判断是否已经接收完毕
    if(pFileInfo->filePos == pFileInfo->fileSize) {
        //关闭文件并回收节点
        QMessageBox::about(pChatDlg, "提示", QString("文件【%1】接收完成").arg(pFileInfo->fileName));
        fclose(pFileInfo->pFile);
        pFileInfo->pFile = nullptr;
        delete pFileInfo;
        pFileInfo = nullptr;
        m_idToFile[fileId] = nullptr;
        m_idToFile.remove(fileId);
    }
}
//处理文件块回复
void CKernel::dealFileBlockRs(char *buf, int nLen, long lSend)
{
    //qDebug()<<__func__;
    //拆包
    STRU_TCP_FILE_BLOCK_RS* rs = (STRU_TCP_FILE_BLOCK_RS*)buf;
    QString fileId = gb2312ToUtf8(rs->fileId);
    STRU_FILE_INFO* pFileInfo = m_idToFile[fileId];
    ChatDialog* pChatDlg = m_idToChatDlgMap[pFileInfo->friendId];
    if(!pFileInfo || !pChatDlg) {
        qDebug()<<"读取文件信息或用户窗口出现错误";
        return;
    }

    pFileInfo->filePos = rs->filePos;

    int value = ((long double)rs->filePos / (pFileInfo->fileSize)) * 100;
    //修改进度条占比
    pChatDlg->updataProcessBar(fileId ,value);

    if(rs->filePos < pFileInfo->fileSize) {
        //偏移文件指针
        _fseeki64(pFileInfo->pFile, rs->filePos, SEEK_SET);
        //读取文件进行发送
        STRU_TCP_FILE_BLOCK_RQ rq;

        rq.blockSize = fread(rq.block,sizeof(char), DEF_FILE_BLOCK_SIZE, pFileInfo->pFile);
        strcpy(rq.fileId, rs->fileId);
        rq.filePos = rs->filePos;

        m_pMediator->SendData((char*)&rq, sizeof(rq), 0);
    }else {
        //关闭文件
        QMessageBox::about(pChatDlg, "提示", QString("文件【%1】发送完成").arg(pFileInfo->fileName));
        fclose(pFileInfo->pFile);
        pFileInfo->pFile = nullptr;
        //回收该节点
        delete pFileInfo;
        pFileInfo = nullptr;
        m_idToFile.remove(fileId);
    }

}
//处理文件取消发送/接收回复
void CKernel::dealFileCancelRs(char *buf, int nLen, long lSend)
{
    qDebug()<<__func__;
    //拆包
    STRU_TCP_FILE_CANCEL_RS* rs = (STRU_TCP_FILE_CANCEL_RS*)buf;

    if(rs->result == cancel_success) {
        //关闭文件，回收节点
        QString fileId = gb2312ToUtf8(rs->fileId);
        auto ite = m_idToFile.find(fileId);
        fclose((*ite)->pFile);
        delete (*ite);
        (*ite) = nullptr;
        m_idToFile.erase(ite);
    }
}
//绑定协议映射表
void CKernel::bindProtocolMap()
{
    memset(m_netProtocolMap, 0, DEF_PROTOCOL_COUNT);
    NetProtocolMap(_DEF_TCP_REGISTER_RS) = &CKernel::dealRegisterRs;
    NetProtocolMap(_DEF_TCP_LOGIN_RS) = &CKernel::dealLoginRs;
    NetProtocolMap(_DEF_TCP_FRIEND_INFO) = &CKernel::dealFriendInfo;
    NetProtocolMap(_DEF_TCP_CHAT_RQ) = &CKernel::dealChatRq;
    NetProtocolMap(_DEF_TCP_OFFLINE_RQ) = &CKernel::dealOfflineRq;
    NetProtocolMap(_DEF_TCP_ADDFRIEND_RQ) = &CKernel::dealAddFriendRq;
    NetProtocolMap(_DEF_TCP_ADDFRIEND_RS) = &CKernel::dealAddFriendRs;
    NetProtocolMap(_DEF_TCP_UPDATA_RQ) = &CKernel::dealUserUpdataRq;
    NetProtocolMap(_DEF_TCP_DELETEFRIEND_RQ) = &CKernel::dealDeleteFriendRq;
    NetProtocolMap(_DEF_TCP_DELETEFRIEND_RS) = &CKernel::dealDeleteFriendRs;
    NetProtocolMap(_DEF_TCP_DELETEFRIEND_RS) = &CKernel::dealDeleteFriendRs;

    //群聊部分
    NetProtocolMap(_DEF_TCP_CREATEGROUP_RS) = &CKernel::dealCreateGroupRs;
    NetProtocolMap(_DEF_TCP_GROUP_INFO) = &CKernel::dealGroupInfo;
    NetProtocolMap(_DEF_TCP_GROUP_MEMBER_INFO) = &CKernel::dealGroupMemberInfo;
    NetProtocolMap(_DEF_TCP_JOINGROUP_RS) = &CKernel::dealJoinGroupRs;
    NetProtocolMap(_DEF_TCP_GROUP_MEMBER_OFFLINE_RQ) = &CKernel::dealGroupMemberOfflineRq;
    NetProtocolMap(_DEF_TCP_MEMBER_DELETE_RQ) = &CKernel::dealGroupDeleteRq;
    NetProtocolMap(_DEF_TCP_MEMBER_EXIT_RQ) = &CKernel::dealGroupExitRq;
    NetProtocolMap(_DEF_TCP_GROUP_CHAT_RQ) = &CKernel::dealGroupMsg;

    //传输文件部分
    NetProtocolMap(_DEF_TCP_SENDFILE_RQ) = &CKernel::dealFriendSendFileRq;
    NetProtocolMap(_DEF_TCP_SENDFILE_RS) = &CKernel::dealFriendSendFileRs;
    NetProtocolMap(_DEF_TCP_FILE_BLOCK_RQ) = &CKernel::dealFileBlockRq;
    NetProtocolMap(_DEF_TCP_FILE_BLOCK_RS) = &CKernel::dealFileBlockRs;


}
//计算MD5哈希值
QString CKernel::calculateMD5(const QString &password)
{
    QByteArray inputData = password.toUtf8();
    QByteArray hashData = QCryptographicHash::hash(inputData, QCryptographicHash::Md5);

    QString md5 = hashData.toHex();
    return md5;
}
//UTF-8(QString)转gb2312(char*)，buf是输出参数，传入的是要写入转码后的空间的起始地址，nLen是空间的长度，utf8是UTF-8编码格式的数据
void CKernel::utf8ToGb2312(char *buf, int nLen, QString utf8)
{
    QTextCodec* gb2312Code = QTextCodec::codecForName("gb2312");
    QByteArray ba = gb2312Code->fromUnicode(utf8);
    strcpy_s(buf, nLen, ba.data());
}
//gb2312转UTF-8，返回值是是转码后的UTF-8编码格式的数据，参数是gb2312编码格式的字符串
QString CKernel::gb2312ToUtf8(char *buf)
{
    QTextCodec* gb2312Code = QTextCodec::codecForName("gb2312");
    return gb2312Code->toUnicode(buf);
}
//加载本地文件
void CKernel::loadingLocationFile()
{
    //创建该用户的本地文件夹
    QString path = QString("D:/Client/%1").arg(m_userId);
    m_userPath = path;
    QDir().mkdir(path);

    //打开文件
    QString fileName = path + QString("/file_%1.data").arg(m_userId);
    FILE* pFile = fopen(fileName.toStdString().c_str(),"r+b");
    if(pFile == nullptr) {
        //创建本地缓存文件
        pFile = fopen(fileName.toStdString().c_str(),"wb");
    }
    _fseeki64(pFile,0,SEEK_END);
    int size = ftell(pFile);
    _fseeki64(pFile,0,SEEK_SET);
    int offSet=0;
    while(offSet < size) {
        STRU_FILE_INFO* info = new STRU_FILE_INFO;
        fread(info,sizeof(STRU_FILE_INFO), 1, pFile);
        m_idToFile[info->fileId] = info;

        if(info->kind == filekind_recv) {
            //断点续传接收文件
            info->status = sendfile_pause;
            info->pFile = fopen(info->filePATH, "r+b");
        }
        else {
            //断点续传上传文件
            info->pFile = fopen(info->filePATH, "rb");
        }
        offSet += sizeof(STRU_FILE_INFO);
    }
    fclose(pFile);
}
//回收资源
void CKernel::deleteRes()
{
    qDebug()<<__func__;
    if(m_pMediator) {
        m_pMediator->CloseNet();
        delete m_pMediator;
        m_pMediator = nullptr;
    }
    auto iteItem = m_idToItemMap.begin();
    while(iteItem != m_idToItemMap.end()) {
        if((*iteItem)) {
            delete (*iteItem);
            (*iteItem) = nullptr;
            iteItem = m_idToItemMap.erase(iteItem);
        }else iteItem++;
    }
    m_idToItemMap.clear();

    auto iteChatDlg = m_idToChatDlgMap.begin();
    while(iteChatDlg != m_idToChatDlgMap.end()) {
        if((*iteChatDlg)) {
            delete (*iteChatDlg);
            (*iteChatDlg) = nullptr;
            iteChatDlg = m_idToChatDlgMap.erase(iteChatDlg);
        }else iteChatDlg++;
    }
    m_idToChatDlgMap.clear();

    auto iteGroupItem = m_idToGroupItemMap.begin();
    while(iteGroupItem != m_idToGroupItemMap.end()) {
        if((*iteGroupItem)) {
            delete (*iteGroupItem);
            (*iteGroupItem) = nullptr;
            iteGroupItem = m_idToGroupItemMap.erase(iteGroupItem);
        }else iteGroupItem++;
    }
    m_idToGroupItemMap.clear();

    auto iteGroupDlgItem = m_idToGroupDlgMap.begin();
    while(iteGroupDlgItem != m_idToGroupDlgMap.end()) {
        if((*iteGroupDlgItem)) {
            delete (*iteGroupDlgItem);
            (*iteGroupDlgItem) = nullptr;
            iteGroupDlgItem = m_idToGroupDlgMap.erase(iteGroupDlgItem);
        }else iteGroupDlgItem++;
    }
    m_idToGroupDlgMap.clear();

    if(m_pMyChat) {
        m_pMyChat->hide();
        delete m_pMyChat;
        m_pMyChat = nullptr;
    }
    //将没上传完和没下载完的文件缓存到本地磁盘中
    //打开文件
    QString fileName = m_userPath + QString("/file_%2.data").arg(m_userId);
    FILE* pFile = fopen(fileName.toStdString().c_str(),"wb");
    auto iteFileInfo = m_idToFile.begin();
    while(iteFileInfo != m_idToFile.end()) {
        if(*iteFileInfo) {
            //关闭当前文件
            if((*iteFileInfo)->pFile) {
                fclose((*iteFileInfo)->pFile);
            }
            //将信息写入缓存文件
            fwrite(*iteFileInfo, sizeof(STRU_FILE_INFO), 1, pFile);
            //回收资源
            delete (*iteFileInfo);
            (*iteFileInfo) = nullptr;
            iteFileInfo = m_idToFile.erase(iteFileInfo);
        }else iteFileInfo++;
    }
    fclose(pFile);
}
//创建指定大小文件
FILE *CKernel::CreateLargeFile(char *filePath, unsigned long long fileSize)
{
    //判断该文件是否存在
        FILE* file = fopen(filePath, "r");
        if (file == nullptr) {
            //不存在 以写方式打开
            file = fopen(filePath, "wb");
            if (file == NULL)
                return nullptr;

            // 将文件指针定位到指定大小处
            int result = _fseeki64(file, fileSize - 1, SEEK_SET);
            if (result != 0)
            {
                fclose(file);
                return nullptr;
            }
            // 写入一个字节以扩展文件大小
            char byte = 0;
            result = fwrite(&byte, sizeof(char), 1, file);
            if (result != 1)
            {
                fclose(file);
                return nullptr;
            }
        }
        //存在 使用读写模式打开
        fclose(file);
        file = fopen(filePath, "r+b");

        return file;
}
