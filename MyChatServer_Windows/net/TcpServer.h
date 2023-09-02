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
	//初始化网络
	virtual bool InitNet();
	//关闭网络
	virtual bool UninitNet();
	//发送数据
	virtual bool SendData(char* buf, int nLen, long lSend);
public:
	static map<unsigned int, SOCKET> m_mapThreadIdToSocket;	//存储线程对应的sock
	static CLock m_threadLock;		//互斥锁和读写锁
private:
	//接受数据
	virtual void RecvData();
	static unsigned __stdcall acceptClientThread(void* lpVoid);
	static unsigned __stdcall recvDataThread(void* lpVoid);
	SOCKET m_serverSock;
	list<HANDLE> m_lstHandle;	//存储线程句柄
	
	bool m_isStop;
};