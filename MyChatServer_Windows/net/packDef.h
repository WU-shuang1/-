#pragma once

#define DEF_TCP_PORT	        (54734)
#define DEF_UDP_PORT	        (54367)
#define DEF_RECV_BUF_SIZE	    (4096)      //udp��Ҫʹ�õġ�
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

// �������궨��
// ע����
#define register_success	    (0)
#define tel_is_exist		    (1)

// ��½���
#define login_success		    (0)
#define password_error		    (1)
#define tel_error			    (2)
#define had_login               (3)

//����״̬
#define status_online           (0)
#define status_offline          (1)

//��Ӻ��ѽ��
#define addfriend_success       (0)
#define addfriend_not_exist     (1)
#define addfriend_refuse        (2)
#define addfriend_offline       (3)
#define addfriend_mine          (4)

//ɾ�����ѽ��
#define deletefriend_success    (0)
#define deletefriend_failed     (1)

//����Ⱥ�Ľ��
#define creategroup_success     (0)
#define creategroup_failed      (1)

//����Ⱥ���
#define joingroup_success       (0)
#define joingroup_not_exist     (1)

//�����ļ����
#define sendfile_success        (0)
#define sendfile_failed         (1)

//�ļ���ͣ��������ȡ������/����
#define sendfile_pause          (0)
#define sendfile_continue       (1)
#define sendfile_cancel         (2)

//ȡ������/���ս��
#define cancel_success          (true)
#define cancel_failed           (false)

//�ļ�����
#define filekind_send           (false)
#define filekind_recv           (true)

//����TCPЭ��ͷ
#define _DEF_PROTOCOL_BASE      (1000)

//ע������
#define _DEF_TCP_REGISTER_RQ                (_DEF_PROTOCOL_BASE+1)
//ע��ظ�
#define _DEF_TCP_REGISTER_RS                (_DEF_PROTOCOL_BASE+2)

//��¼����
#define _DEF_TCP_LOGIN_RQ                   (_DEF_PROTOCOL_BASE+3)
//��¼�ظ�
#define _DEF_TCP_LOGIN_RS                   (_DEF_PROTOCOL_BASE+4)
//������Ϣ
#define _DEF_TCP_FRIEND_INFO                (_DEF_PROTOCOL_BASE+5)
//��������
#define _DEF_TCP_CHAT_RQ                    (_DEF_PROTOCOL_BASE+6)
//��������
#define _DEF_TCP_OFFLINE_RQ                 (_DEF_PROTOCOL_BASE+7)
//��Ӻ�������
#define _DEF_TCP_ADDFRIEND_RQ               (_DEF_PROTOCOL_BASE+8)
//��Ӻ��ѻظ�
#define _DEF_TCP_ADDFRIEND_RS               (_DEF_PROTOCOL_BASE+9)
//ɾ����������    
#define _DEF_TCP_DELETEFRIEND_RQ            (_DEF_PROTOCOL_BASE+10)
//�޸ĸ�����Ϣ����
#define _DEF_TCP_UPDATA_RQ                  (_DEF_PROTOCOL_BASE+11)
//ɾ�����ѻظ�
#define _DEF_TCP_DELETEFRIEND_RS            (_DEF_PROTOCOL_BASE+12)

//����Ⱥ������
#define _DEF_TCP_CREATEGROUP_RQ             (_DEF_PROTOCOL_BASE+13)
//����Ⱥ�Ļظ�
#define _DEF_TCP_CREATEGROUP_RS             (_DEF_PROTOCOL_BASE+14)
//����Ⱥ������
#define _DEF_TCP_JOINGROUP_RQ               (_DEF_PROTOCOL_BASE+15)
//����Ⱥ�Ļظ�
#define _DEF_TCP_JOINGROUP_RS               (_DEF_PROTOCOL_BASE+16)
//Ⱥ��������
#define _DEF_TCP_GROUP_CHAT_RQ              (_DEF_PROTOCOL_BASE+17)
//Ⱥ����Ϣ
#define _DEF_TCP_GROUP_INFO                 (_DEF_PROTOCOL_BASE+18)
//Ⱥ��Ա��Ϣ
#define _DEF_TCP_GROUP_MEMBER_INFO          (_DEF_PROTOCOL_BASE+19)
//Ⱥ��Ա��������
#define _DEF_TCP_GROUP_MEMBER_OFFLINE_RQ    (_DEF_PROTOCOL_BASE+20)
//��ɢȺ������
#define _DEF_TCP_MEMBER_DELETE_RQ           (_DEF_PROTOCOL_BASE+21)
//�˳�Ⱥ������
#define _DEF_TCP_MEMBER_EXIT_RQ             (_DEF_PROTOCOL_BASE+22)

//�����ļ�����
#define _DEF_TCP_SENDFILE_RQ                (_DEF_PROTOCOL_BASE+23)
//�����ļ�����ظ�
#define _DEF_TCP_SENDFILE_RS                (_DEF_PROTOCOL_BASE+24)

//�ļ�������
#define _DEF_TCP_FILE_BLOCK_RQ              (_DEF_PROTOCOL_BASE+25)
//�ļ���ظ�
#define _DEF_TCP_FILE_BLOCK_RS              (_DEF_PROTOCOL_BASE+26)
//�ļ�ȡ����������
#define _DEF_TCP_FILE_CANCEL_RQ             (_DEF_PROTOCOL_BASE+27)
#define _DEF_TCP_FILE_CANCEL_RS             (_DEF_PROTOCOL_BASE+29)

//�ļ���ͣ����������/����
#define _DEF_TCP_FILE_PAUSE_RQ              (_DEF_PROTOCOL_BASE+28)

//TCP����ṹ��
typedef int PackType;
//ע������Э��ͷ���ֻ��š�����
struct STRU_TCP_REGISTER_RQ {
    STRU_TCP_REGISTER_RQ() : type(_DEF_TCP_REGISTER_RQ) {
        memset(tel, 0, DEF_TEL_SIZE);
        memset(password, 0, DEF_PASSWORD_SIZE);
    }
    PackType type;
    char tel[DEF_TEL_SIZE];
    char password[DEF_PASSWORD_SIZE];
};
//ע��ظ���Э��ͷ��ע����
struct STRU_TCP_REGISTER_RS {
    STRU_TCP_REGISTER_RS() : type(_DEF_TCP_REGISTER_RS) {

    }
    PackType type;
    int result;
};
//��¼����Э��ͷ���ֻ��š�����
struct STRU_TCP_LOGIN_RQ {
    STRU_TCP_LOGIN_RQ() : type(_DEF_TCP_LOGIN_RQ) {
        memset(tel, 0, DEF_TEL_SIZE);
        memset(password, 0, DEF_PASSWORD_SIZE);
    }
    PackType type;
    char tel[DEF_TEL_SIZE];
    char password[DEF_PASSWORD_SIZE];
};
//��¼�ظ���Э��ͷ����¼������û�id
struct STRU_TCP_LOGIN_RS {
    STRU_TCP_LOGIN_RS() : type(_DEF_TCP_LOGIN_RS), result(0), userId(0) {

    }
    PackType type;
    int result;
    int userId;
};
//������Ϣ��Э��ͷ������id��ͷ��id������״̬�������ǳơ�����ǩ��
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

//��������Э��ͷ������id������id����������
struct STRU_TCP_CHAT_RQ {
    STRU_TCP_CHAT_RQ() :type(_DEF_TCP_CHAT_RQ) {
        memset(content, 0, DEF_CONTENT_SIZE);
    }
    PackType type;
    int userId;
    int friendId;
    char content[DEF_CONTENT_SIZE];
};
//��������Э��ͷ���û�id
struct STRU_TCP_OFFLINE_RQ {
    STRU_TCP_OFFLINE_RQ() :type(_DEF_TCP_OFFLINE_RQ) {

    }
    PackType type;
    int userId;
};
//��Ӻ�������Э��ͷ���û�id���û��ǳơ������ֻ���
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
//��Ӻ��ѻظ���Э��ͷ����ӽ�����û�id���û��ǳơ�����id
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
//ɾ����������Э��ͷ���û�id������id
struct STRU_TCP_DELETEFRIEND_RQ {
    STRU_TCP_DELETEFRIEND_RQ() :type(_DEF_TCP_DELETEFRIEND_RQ) {

    }
    PackType type;
    int userId;
    int friendId;
};
//ɾ�����ѻظ���Э��ͷ������id�����
struct STRU_TCP_DELETEFRIEND_RS {
    STRU_TCP_DELETEFRIEND_RS() :type(_DEF_TCP_DELETEFRIEND_RS) {

    }
    PackType type;
    int friendId;
    int result;
};

//�޸ĸ�����Ϣ����Э��ͷ���û�id���û��ǳơ��û�ǩ����ͷ��id
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

//����Ⱥ������Э��ͷ���û�id��Ⱥ������
struct STRU_TCP_CREATEGROUP_RQ
{
    STRU_TCP_CREATEGROUP_RQ() :type(_DEF_TCP_CREATEGROUP_RQ) {
        memset(groupName, 0, DEF_NAME_SIZE);
    }
    PackType type;
    int userId;
    char groupName[DEF_NAME_SIZE];
};
//����Ⱥ�Ļظ���Э��ͷ���������
struct STRU_TCP_CREATEGROUP_RS
{
    STRU_TCP_CREATEGROUP_RS() :type(_DEF_TCP_CREATEGROUP_RS) {
        
    }
    PackType type;
    int result;
};
//����Ⱥ������Э��ͷ���û�id��Ⱥ��id
struct STRU_TCP_JOINGROUP_RQ
{
    STRU_TCP_JOINGROUP_RQ() :type(_DEF_TCP_JOINGROUP_RQ) {

    }
    PackType type;
    int userId;
    int groupId;
};
//����Ⱥ�Ļظ���Э��ͷ��������
struct STRU_TCP_JOINGROUP_RS
{
    STRU_TCP_JOINGROUP_RS() :type(_DEF_TCP_JOINGROUP_RS) {

    }
    PackType type;
    int result;
};
//Ⱥ��������Э��ͷ���û�id��Ⱥ��id����������
struct STRU_TCP_GROUP_CHAT_RQ
{
    STRU_TCP_GROUP_CHAT_RQ() :type(_DEF_TCP_GROUP_CHAT_RQ) {

    }
    PackType type;
    int userId;
    int groupId;
    char content[DEF_CONTENT_SIZE];
};
//Ⱥ����Ϣ��Э��ͷ��Ⱥ��id��Ⱥ��id��Ⱥ�����֡�Ⱥ��ͷ��id��������
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
//Ⱥ��Ա��Ϣ��Э��ͷ��Ⱥ��id����Աid����Աͷ��id����Ա���֡���Ա����״̬
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
//Ⱥ��Ա��������Э��ͷ��Ⱥ��id���û�id
struct STRU_TCP_GROUP_MEMBER_OFFLINE_RQ {
    STRU_TCP_GROUP_MEMBER_OFFLINE_RQ() :type(_DEF_TCP_GROUP_MEMBER_OFFLINE_RQ) {

    }
    PackType type;
    int groupId;
    int userId;
};
//�˳�\��ɢȺ����Э��ͬ��Ⱥ��id���û�id
struct STRU_TCP_GROUP_DELETE_RQ {
    STRU_TCP_GROUP_DELETE_RQ() :type(_DEF_TCP_MEMBER_DELETE_RQ) {

    }
    PackType type;
    int groupId;
    int userId;
};

//�����ļ�����Э��ͷ���û�id������id���ļ����֡��ļ�id���ļ���С
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
//�����ļ��ظ���Э��ͷ���ļ�id������id��������
struct STRU_TCP_SEND_FILE_RS {
    STRU_TCP_SEND_FILE_RS() :type(_DEF_TCP_SENDFILE_RS) {
        memset(fileId, 0, DEF_FILE_ID_SIZE);
    }
    PackType type;
    int friendId;
    int result;
    char fileId[DEF_FILE_ID_SIZE];
};
//�ļ�������Э��ͷ���ļ�id���ļ����С�������ݡ�ָ��λ��
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
//�ļ���ظ���Э��ͷ���ļ�id������id��������
struct STRU_TCP_FILE_BLOCK_RS {
    STRU_TCP_FILE_BLOCK_RS() :type(_DEF_TCP_FILE_BLOCK_RS), filePos(0){

    }
    PackType type;
    int friendId;
    unsigned long long filePos;
    char fileId[DEF_FILE_ID_SIZE];
};

//�ļ���Ϣ���û�id������id���ļ�id���ļ������ļ���С���洢·������ǰλ�á��ļ�ָ�롢�ļ�����״̬����ͣ��������
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
//ȡ���ļ�����/��������Э��ͷ���ļ�id
struct STRU_TCP_FILE_CANCEL_RQ {
    STRU_TCP_FILE_CANCEL_RQ() :type(_DEF_TCP_FILE_CANCEL_RQ) {
        memset(fileId, 0, DEF_FILE_ID_SIZE);
    }
    PackType type;
    char fileId[DEF_FILE_ID_SIZE];
};
//ȡ���ļ�����/��������Э��ͷ���ļ�id��������
struct STRU_TCP_FILE_CANCEL_RS {
    STRU_TCP_FILE_CANCEL_RS() :type(_DEF_TCP_FILE_CANCEL_RS) {
        memset(fileId, 0, DEF_FILE_ID_SIZE);
    }
    PackType type;
    bool result;
    char fileId[DEF_FILE_ID_SIZE];
};
//��ͣ�������ļ�����/��������Э��ͷ���ļ�id���ļ�����״̬����ͣ�����������ļ����ࣨ�ϴ������أ�
struct STRU_TCP_FILE_PAUSE_RQ {
    STRU_TCP_FILE_PAUSE_RQ() :type(_DEF_TCP_FILE_PAUSE_RQ),kind(filekind_send) {
        memset(fileId, 0, DEF_FILE_ID_SIZE);
    }
    PackType type;
    bool flag;
    bool kind;
    char fileId[DEF_FILE_ID_SIZE];
};