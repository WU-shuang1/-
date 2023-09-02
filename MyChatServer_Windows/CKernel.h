#pragma once
#include <iostream>
#include"TcpServerMediator.h"
#include "CMySql.h"
#include "pch.h"
#include "packDef.h"
#include <map>
using namespace std;

class CKernel;
typedef void (CKernel::*pfun) (char*, int, long);

class CKernel
{
private:
	CKernel();
	CKernel(const CKernel& other){}
	~CKernel();
public:
	//�򿪷�����
	bool startServer();
	//�رշ�����
	void closeServer();

	//������յ�����������
	void dealData(char* buf, int nLen, long lSend);
	//����ע������
	void dealRegisterRq(char* buf, int nLen, long lSend);
	//�����¼����
	void dealLoginRq(char* buf, int nLen, long lSend);
	//������������
	void dealChatRq(char* buf, int nLen, long lSend);
	//������������
	void dealOfflineRq(char* buf, int nLen, long lSend);
	//������Ӻ�������
	void dealAddFriendRq(char* buf, int nLen, long lSend);
	//������Ӻ��ѻظ�
	void dealAddFriendRs(char* buf, int nLen, long lSend);
	//����ɾ����������
	void dealDeleteFriendRq(char* buf, int nLen, long lSend);
	//��������û���Ϣ����
	void dealUserUpdataRq(char* buf, int nLen, long lSend);
	
	//������Ⱥ������
	void dealCreateGroupRq(char* buf, int nLen, long lSend);
	//�������Ⱥ������
	void dealJoinGroupRq(char* buf, int nLen, long lSend);
	//�������Ⱥ����Ϣ����
	void dealGroupUpDataInfoRq(char* buf, int nLen, long lSend);
	//�����ɢȺ������
	void dealGroupDeleteRq(char* buf, int nLen, long lSend);
	//�����˳�Ⱥ������
	void dealGroupExitRq(char* buf, int nLen, long lSend);
	//����Ⱥ����Ϣ
	void dealGroupMsg(char* buf, int nLen, long lSend);

	//������ѷ����ļ�����
	void dealFriendSendFileRq(char* buf, int nLen, long lSend);
	//�������ļ��ظ�
	void dealFriendSendFileRs(char* buf, int nLen, long lSend);

	//������ѷ��͵����ݿ�
	void dealFileBlockRq(char* buf, int nLen, long lSend);
	//�����ļ���ظ�
	void dealFileBlockRs(char* buf, int nLen, long lSend);

	//�����޸��ļ�״̬
	void dealFileStatusRq(char* buf, int nLen, long lSend);
	//����ȡ������/�����ļ�
	void dealFileCancelRq(char* buf, int nLen, long lSend);

	//��ȡ���к�����Ϣ�������Լ���
	void GetAllFriendInfo(int userId);
	//��ȡ�û���Ϣ
	void GetUserInfo(STRU_TCP_FRIEND_INFO* info, int userId);
	//��ȡ����Ⱥ����Ϣ
	void GetAllGroupInfo(int userId);
	//��ȡȺ�ĵ���Ϣ
	void GetGroupInfo(STRU_TCP_GROUOP_INFO* info, int groupId);
	//��ȡ����Ⱥ��Ա��Ϣ
	void GetAllGroupMemberInfo(int groupId ,int userId);
	//��ȡĳ��Ⱥ��Ա��Ϣ
	void GetGroupMemberInfo(STRU_TCP_GROUOP_MEMBER_INFO* info, int userId);

	//��ȡ�����ڷ���������Ҫ���յ��ļ�
	void GetAllFile(int userId);

	//��Э��ӳ���
	void bindProtocolMap();
	//��ȡkernel����
	static CKernel* GetKernel();
	//��ȡ�����
	string GetSalt(int len);
	//����ָ����С�ļ�
	FILE* CreateLargeFile(char* filePath, unsigned long long fileSize);
	
public:
	CLock m_lock;		//�ļ�id-�ļ���Ϣ��map�Ķ�д��
	CMD5Encrypt m_md5;
	INetMediator* m_pMediator;
	CMySql* m_mysql;
	map <int, SOCKET> m_mapIdToSocket;		//ʹ��map�����û���sock�������û�idΪkey��sockΪvalue
	pfun m_netProtocolMap[DEF_PROTOCOL_COUNT];			//����Э��ӳ���
	map <string, STRU_FILE_INFO*> m_FileIdToInfo;	//�����ļ���Ϣ
private:
	static CKernel m_kernel;
};

