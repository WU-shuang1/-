#pragma once
#include"INet.h"

class TcpClient:public INet
{
public:
	TcpClient(INetMediator* mediator);
	~TcpClient();

public:
	//��ʼ������
	virtual bool InitNet();
	//�ر�����
	virtual bool UninitNet();
	//��������
	virtual bool SendData(char* buf, int nLen, long lSend);
private:
	//��������
	virtual void RecvData();
	static unsigned __stdcall recvDataThread(void* lpVoid);
	SOCKET m_clientSock;
	HANDLE m_handle;
	bool m_isStop;
};


