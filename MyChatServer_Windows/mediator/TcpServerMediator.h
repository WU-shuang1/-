#pragma once
#include"INetMediator.h"
class CKernel;
class TcpServerMediator :public INetMediator
{
public:
	TcpServerMediator();
	~TcpServerMediator();

public:
	//初始化网络
	virtual bool OpenNet();
	//关闭网络
	virtual bool CloseNet();
	//发送数据
	virtual bool SendData(char* buf, int nLen, long lSend);
	//接受数据
	virtual void DealData(char* buf, int nLen, long lSend);
private:
	CKernel* m_pKernel;
};