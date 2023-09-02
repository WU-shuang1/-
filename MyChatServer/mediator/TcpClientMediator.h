#pragma once
#include"INetMediator.h"

class TcpClientMediator :public INetMediator
{
public:
	TcpClientMediator();
	~TcpClientMediator();

public:
	//初始化网络
	virtual bool OpenNet();
	//关闭网络
	virtual bool CloseNet();
	//发送数据
	virtual bool SendData(char* buf, int nLen, long lSend);
	//接受数据
	virtual void DealData(char* buf, int nLen, long lSend);
};

