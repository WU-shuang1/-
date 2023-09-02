#pragma once
#include"INet.h"
#include "Clock.h"
#include <map>
#include <list>
class TcpServer :public INet
{
public:
	TcpServer(INetMediator* mediator);
	~TcpServer();

public:
	//��ʼ������
	virtual bool InitNet();
	//�ر�����
	virtual bool UninitNet();
	//��������
	virtual bool SendData(char* buf, int nLen, long lSend);
public:
	static map<unsigned int, SOCKET> m_mapThreadIdToSocket;	//�洢�̶߳�Ӧ��sock
	static CLock m_threadLock;		//�������Ͷ�д��
private:
	//��������
	virtual void RecvData();
	static unsigned __stdcall acceptClientThread(void* lpVoid);
	static unsigned __stdcall recvDataThread(void* lpVoid);
	SOCKET m_serverSock;
	list<HANDLE> m_lstHandle;	//�洢�߳̾��
	
	bool m_isStop;
};