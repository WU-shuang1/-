#pragma once

#define DEF_TCP_PORT	        (54734)
#define DEF_UDP_PORT	        (54367)
#define DEF_RECV_BUF_SIZE	    (4096)      //udp需要使用的·
#define DEF_TEL_SIZE            (12)
#define DEF_PASSWORD_SIZE       (33)
#define DEF_SALT_SIZE           (10)
#define DEF_NAME_SIZE           (60)
#define DEF_FEELING_SIZE        (120)
#define DEF_CONTENT_SIZE        (2048)

#define DEF_FILE_NAME_SIZE      (512)
#define DEF_FILE_ID_SIZE        (512)
#define DEF_FILE_BLOCK_SIZE     (8*1024)

#define DEF_PROTOCOL_COUNT      (29)

// 请求结果宏定义
// 注册结果
#define register_success	    (0)
#define tel_is_exist		    (1)

// 登陆结果
#define login_success		    (0)
#define password_error		    (1)
#define tel_error			    (2)
#define had_login               (3)

//在线状态
#define status_online           (0)
#define status_offline          (1)

//添加好友结果
#define addfriend_success       (0)
#define addfriend_not_exist     (1)
#define addfriend_refuse        (2)
#define addfriend_offline       (3)
#define addfriend_mine          (4)

//删除好友结果
#define deletefriend_success    (0)
#define deletefriend_failed     (1)

//创建群聊结果
#define creategroup_success     (0)
#define creategroup_failed      (1)

//加入群结果
#define joingroup_success       (0)
#define joingroup_not_exist     (1)

//发送文件结果
#define sendfile_success        (0)
#define sendfile_failed         (1)

//文件暂停、继续、取消接收/发送
#define sendfile_pause          (0)
#define sendfile_continue       (1)
#define sendfile_cancel         (2)

//取消发送/接收结果
#define cancel_success          (true)
#define cancel_failed           (false)

//文件种类
#define filekind_send           (false)
#define filekind_recv           (true)

//定义TCP协议头
#define _DEF_PROTOCOL_BASE      (1000)

//注册请求
#define _DEF_TCP_REGISTER_RQ                (_DEF_PROTOCOL_BASE+1)
//注册回复
#define _DEF_TCP_REGISTER_RS                (_DEF_PROTOCOL_BASE+2)

//登录请求
#define _DEF_TCP_LOGIN_RQ                   (_DEF_PROTOCOL_BASE+3)
//登录回复
#define _DEF_TCP_LOGIN_RS                   (_DEF_PROTOCOL_BASE+4)
//好友信息
#define _DEF_TCP_FRIEND_INFO                (_DEF_PROTOCOL_BASE+5)
//聊天请求
#define _DEF_TCP_CHAT_RQ                    (_DEF_PROTOCOL_BASE+6)
//下线请求
#define _DEF_TCP_OFFLINE_RQ                 (_DEF_PROTOCOL_BASE+7)
//添加好友请求
#define _DEF_TCP_ADDFRIEND_RQ               (_DEF_PROTOCOL_BASE+8)
//添加好友回复
#define _DEF_TCP_ADDFRIEND_RS               (_DEF_PROTOCOL_BASE+9)
//删除好友请求    
#define _DEF_TCP_DELETEFRIEND_RQ            (_DEF_PROTOCOL_BASE+10)
//修改个人信息请求
#define _DEF_TCP_UPDATA_RQ                  (_DEF_PROTOCOL_BASE+11)
//删除好友回复
#define _DEF_TCP_DELETEFRIEND_RS            (_DEF_PROTOCOL_BASE+12)

//创建群聊请求
#define _DEF_TCP_CREATEGROUP_RQ             (_DEF_PROTOCOL_BASE+13)
//创建群聊回复
#define _DEF_TCP_CREATEGROUP_RS             (_DEF_PROTOCOL_BASE+14)
//加入群聊请求
#define _DEF_TCP_JOINGROUP_RQ               (_DEF_PROTOCOL_BASE+15)
//加入群聊回复
#define _DEF_TCP_JOINGROUP_RS               (_DEF_PROTOCOL_BASE+16)
//群聊天请求
#define _DEF_TCP_GROUP_CHAT_RQ              (_DEF_PROTOCOL_BASE+17)
//群聊信息
#define _DEF_TCP_GROUP_INFO                 (_DEF_PROTOCOL_BASE+18)
//群成员信息
#define _DEF_TCP_GROUP_MEMBER_INFO          (_DEF_PROTOCOL_BASE+19)
//群成员下线请求
#define _DEF_TCP_GROUP_MEMBER_OFFLINE_RQ    (_DEF_PROTOCOL_BASE+20)
//解散群聊请求
#define _DEF_TCP_MEMBER_DELETE_RQ           (_DEF_PROTOCOL_BASE+21)
//退出群聊请求
#define _DEF_TCP_MEMBER_EXIT_RQ             (_DEF_PROTOCOL_BASE+22)

//传输文件请求
#define _DEF_TCP_SENDFILE_RQ                (_DEF_PROTOCOL_BASE+23)
//传输文件请求回复
#define _DEF_TCP_SENDFILE_RS                (_DEF_PROTOCOL_BASE+24)

//文件块请求
#define _DEF_TCP_FILE_BLOCK_RQ              (_DEF_PROTOCOL_BASE+25)
//文件块回复
#define _DEF_TCP_FILE_BLOCK_RS              (_DEF_PROTOCOL_BASE+26)
//文件取消传输请求
#define _DEF_TCP_FILE_CANCEL_RQ             (_DEF_PROTOCOL_BASE+27)
#define _DEF_TCP_FILE_CANCEL_RS             (_DEF_PROTOCOL_BASE+29)

//文件暂停、继续发送/接收
#define _DEF_TCP_FILE_PAUSE_RQ              (_DEF_PROTOCOL_BASE+28)

//TCP请求结构体
typedef int PackType;
//注册请求：协议头、手机号、密码
struct STRU_TCP_REGISTER_RQ {
    STRU_TCP_REGISTER_RQ() : type(_DEF_TCP_REGISTER_RQ) {
        memset(tel, 0, DEF_TEL_SIZE);
        memset(password, 0, DEF_PASSWORD_SIZE);
    }
    PackType type;
    char tel[DEF_TEL_SIZE];
    char password[DEF_PASSWORD_SIZE];
};
//注册回复：协议头、注册结果
struct STRU_TCP_REGISTER_RS {
    STRU_TCP_REGISTER_RS() : type(_DEF_TCP_REGISTER_RS) {

    }
    PackType type;
    int result;
};
//登录请求：协议头、手机号、密码
struct STRU_TCP_LOGIN_RQ {
    STRU_TCP_LOGIN_RQ() : type(_DEF_TCP_LOGIN_RQ) {
        memset(tel, 0, DEF_TEL_SIZE);
        memset(password, 0, DEF_PASSWORD_SIZE);
    }
    PackType type;
    char tel[DEF_TEL_SIZE];
    char password[DEF_PASSWORD_SIZE];
};
//登录回复：协议头、登录结果、用户id
struct STRU_TCP_LOGIN_RS {
    STRU_TCP_LOGIN_RS() : type(_DEF_TCP_LOGIN_RS), result(0), userId(0) {

    }
    PackType type;
    int result;
    int userId;
};
//好友信息：协议头、好友id、头像id、在线状态、好友昵称、好友签名
struct STRU_TCP_FRIEND_INFO
{
    STRU_TCP_FRIEND_INFO() :type(_DEF_TCP_FRIEND_INFO), friendId(0), status(0) {
        memset(name, 0, DEF_NAME_SIZE);
        memset(feeling, 0, DEF_FEELING_SIZE);
    }
    PackType type;
    int friendId;
    int iconId;
    int status;
    char name[DEF_NAME_SIZE];
    char feeling[DEF_FEELING_SIZE];
};

//聊天请求：协议头、本人id、好友id、聊天内容
struct STRU_TCP_CHAT_RQ {
    STRU_TCP_CHAT_RQ() :type(_DEF_TCP_CHAT_RQ) {
        memset(content, 0, DEF_CONTENT_SIZE);
    }
    PackType type;
    int userId;
    int friendId;
    char content[DEF_CONTENT_SIZE];
};
//下线请求：协议头、用户id
struct STRU_TCP_OFFLINE_RQ {
    STRU_TCP_OFFLINE_RQ() :type(_DEF_TCP_OFFLINE_RQ) {

    }
    PackType type;
    int userId;
};
//添加好友请求：协议头、用户id、用户昵称、好友手机号
struct STRU_TCP_ADDFRIEND_RQ {
    STRU_TCP_ADDFRIEND_RQ() :type(_DEF_TCP_ADDFRIEND_RQ) {
        memset(userName, 0, DEF_NAME_SIZE);
        memset(friendTel, 0, DEF_TEL_SIZE);
    }
    PackType type;
    int userId;
    char userName[DEF_NAME_SIZE];
    char friendTel[DEF_TEL_SIZE];

};
//添加好友回复：协议头、添加结果、用户id、用户昵称、好友id
struct STRU_TCP_ADDFRIEND_RS {
    STRU_TCP_ADDFRIEND_RS() :type(_DEF_TCP_ADDFRIEND_RS) {
        memset(userName, 0, DEF_NAME_SIZE);
    }
    PackType type;
    int result;
    int userId;
    int friendId;
    char userName[DEF_NAME_SIZE];
};
//删除好友请求：协议头、用户id、好友id
struct STRU_TCP_DELETEFRIEND_RQ {
    STRU_TCP_DELETEFRIEND_RQ() :type(_DEF_TCP_DELETEFRIEND_RQ) {

    }
    PackType type;
    int userId;
    int friendId;
};
//删除好友回复：协议头、好友id、结果
struct STRU_TCP_DELETEFRIEND_RS {
    STRU_TCP_DELETEFRIEND_RS() :type(_DEF_TCP_DELETEFRIEND_RS) {

    }
    PackType type;
    int friendId;
    int result;
};

//修改个人信息请求：协议头、用户id、用户昵称、用户签名、头像id
struct STRU_TCP_UPDATA_RQ
{
    STRU_TCP_UPDATA_RQ() :type(_DEF_TCP_UPDATA_RQ) {
        memset(userName, 0, DEF_NAME_SIZE);
        memset(userFeeling, 0, DEF_FEELING_SIZE);
    }
    PackType type;
    int userId;
    int iconId;
    char userName[DEF_NAME_SIZE];
    char userFeeling[DEF_FEELING_SIZE];
};

//创建群聊请求：协议头、用户id、群聊名字
struct STRU_TCP_CREATEGROUP_RQ
{
    STRU_TCP_CREATEGROUP_RQ() :type(_DEF_TCP_CREATEGROUP_RQ) {
        memset(groupName, 0, DEF_NAME_SIZE);
    }
    PackType type;
    int userId;
    char groupName[DEF_NAME_SIZE];
};
//创建群聊回复：协议头、创建结果
struct STRU_TCP_CREATEGROUP_RS
{
    STRU_TCP_CREATEGROUP_RS() :type(_DEF_TCP_CREATEGROUP_RS) {
        
    }
    PackType type;
    int result;
};
//加入群聊请求：协议头、用户id、群聊id
struct STRU_TCP_JOINGROUP_RQ
{
    STRU_TCP_JOINGROUP_RQ() :type(_DEF_TCP_JOINGROUP_RQ) {

    }
    PackType type;
    int userId;
    int groupId;
};
//加入群聊回复：协议头、加入结果
struct STRU_TCP_JOINGROUP_RS
{
    STRU_TCP_JOINGROUP_RS() :type(_DEF_TCP_JOINGROUP_RS) {

    }
    PackType type;
    int result;
};
//群聊天请求：协议头、用户id、群聊id、聊天内容
struct STRU_TCP_GROUP_CHAT_RQ
{
    STRU_TCP_GROUP_CHAT_RQ() :type(_DEF_TCP_GROUP_CHAT_RQ) {

    }
    PackType type;
    int userId;
    int groupId;
    char content[DEF_CONTENT_SIZE];
};
//群聊信息：协议头、群聊id、群主id、群聊名字、群聊头像id、总人数
struct STRU_TCP_GROUOP_INFO
{
    STRU_TCP_GROUOP_INFO() :type(_DEF_TCP_GROUP_INFO) {
        memset(name, 0, DEF_NAME_SIZE);
    }
    PackType type;
    int groupId;
    int userId;
    int groupIconId;
    int total;
    char name[DEF_NAME_SIZE];
};
//群成员信息：协议头、群聊id、成员id、成员头像id、成员名字、成员在线状态
struct STRU_TCP_GROUOP_MEMBER_INFO
{
    STRU_TCP_GROUOP_MEMBER_INFO() :type(_DEF_TCP_GROUP_MEMBER_INFO) {
        memset(userName, 0, DEF_NAME_SIZE);
    }
    PackType type;
    int groupId;
    int userId;
    int userIconId;
    int status;
    char userName[DEF_NAME_SIZE];
};
//群成员下线请求：协议头、群聊id、用户id
struct STRU_TCP_GROUP_MEMBER_OFFLINE_RQ {
    STRU_TCP_GROUP_MEMBER_OFFLINE_RQ() :type(_DEF_TCP_GROUP_MEMBER_OFFLINE_RQ) {

    }
    PackType type;
    int groupId;
    int userId;
};
//退出\解散群请求：协议同、群聊id、用户id
struct STRU_TCP_GROUP_DELETE_RQ {
    STRU_TCP_GROUP_DELETE_RQ() :type(_DEF_TCP_MEMBER_DELETE_RQ) {

    }
    PackType type;
    int groupId;
    int userId;
};

//发送文件请求：协议头、用户id、好友id、文件名字、文件id、文件大小
struct STRU_TCP_SEND_FILE_RQ {
    STRU_TCP_SEND_FILE_RQ() :type(_DEF_TCP_SENDFILE_RQ), fileSize(0), filePos(0){
        memset(fileId, 0, DEF_FILE_ID_SIZE);
        memset(fileName, 0, DEF_FILE_NAME_SIZE);
    }
    PackType type;
    int userId;
    int friendId;
    unsigned long long fileSize;
    unsigned long long filePos;
    char fileId[DEF_FILE_ID_SIZE];
    char fileName[DEF_FILE_NAME_SIZE];
};
//发送文件回复：协议头、文件id、好友id、请求结果
struct STRU_TCP_SEND_FILE_RS {
    STRU_TCP_SEND_FILE_RS() :type(_DEF_TCP_SENDFILE_RS) {
        memset(fileId, 0, DEF_FILE_ID_SIZE);
    }
    PackType type;
    int friendId;
    int result;
    char fileId[DEF_FILE_ID_SIZE];
};
//文件块请求：协议头、文件id、文件块大小、块内容、指针位置
struct STRU_TCP_FILE_BLOCK_RQ {
    STRU_TCP_FILE_BLOCK_RQ() :type(_DEF_TCP_FILE_BLOCK_RQ), filePos(0){
        memset(block, 0, DEF_FILE_BLOCK_SIZE);
        memset(fileId, 0, DEF_FILE_ID_SIZE);
    }
    PackType type;
    unsigned long long filePos;
    int blockSize;
    
    char block[DEF_FILE_BLOCK_SIZE];
    char fileId[DEF_FILE_ID_SIZE];
};
//文件块回复：协议头、文件id、好友id、下载量
struct STRU_TCP_FILE_BLOCK_RS {
    STRU_TCP_FILE_BLOCK_RS() :type(_DEF_TCP_FILE_BLOCK_RS), filePos(0){

    }
    PackType type;
    int friendId;
    unsigned long long filePos;
    char fileId[DEF_FILE_ID_SIZE];
};

//文件信息：用户id、好友id、文件id、文件名、文件大小、存储路径、当前位置、文件指针、文件下载状态（暂停、继续）
struct STRU_FILE_INFO {
    STRU_FILE_INFO() :pFile(nullptr),status(sendfile_continue),kind(filekind_send), filePos(0) {
        memset(fileId, 0, DEF_FILE_ID_SIZE);
        memset(fileName, 0, DEF_FILE_NAME_SIZE);
        memset(filePATH, 0, DEF_FILE_NAME_SIZE);
    }
    unsigned long long fileSize;
    unsigned long long filePos;
    int userId;
    int friendId;
    FILE* pFile;
    bool status;
    bool kind;
    char fileId[DEF_FILE_ID_SIZE];
    char fileName[DEF_FILE_NAME_SIZE];
    char filePATH[DEF_FILE_NAME_SIZE];
};
//取消文件发送/接收请求：协议头、文件id
struct STRU_TCP_FILE_CANCEL_RQ {
    STRU_TCP_FILE_CANCEL_RQ() :type(_DEF_TCP_FILE_CANCEL_RQ) {
        memset(fileId, 0, DEF_FILE_ID_SIZE);
    }
    PackType type;
    char fileId[DEF_FILE_ID_SIZE];
};
//取消文件发送/接收请求：协议头、文件id、请求结果
struct STRU_TCP_FILE_CANCEL_RS {
    STRU_TCP_FILE_CANCEL_RS() :type(_DEF_TCP_FILE_CANCEL_RS) {
        memset(fileId, 0, DEF_FILE_ID_SIZE);
    }
    PackType type;
    bool result;
    char fileId[DEF_FILE_ID_SIZE];
};
//暂停、继续文件发送/接收请求：协议头、文件id、文件下载状态（暂停、继续）、文件种类（上传、下载）
struct STRU_TCP_FILE_PAUSE_RQ {
    STRU_TCP_FILE_PAUSE_RQ() :type(_DEF_TCP_FILE_PAUSE_RQ),kind(filekind_send) {
        memset(fileId, 0, DEF_FILE_ID_SIZE);
    }
    PackType type;
    bool flag;
    bool kind;
    char fileId[DEF_FILE_ID_SIZE];
};