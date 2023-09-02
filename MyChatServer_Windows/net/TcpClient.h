#pragma once
#include"INet.h"

class TcpClient:public INet
{
public:
	TcpClient(INetMediator* mediator);
	~TcpClient();

public:
	//初始化网络
	virtual bool InitNet();
	//关闭网络
	virtual bool UninitNet();
	//发送数据
	virtual bool SendData(char* buf, int nLen, long lSend);
private:
	//接受数据
	virtual void RecvData();
	static unsigned __stdcall recvDataThread(void* lpVoid);
	SOCKET m_clientSock;
	HANDLE m_handle;
	bool m_isStop;
};


