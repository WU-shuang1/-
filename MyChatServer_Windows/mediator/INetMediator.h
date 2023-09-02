#pragma once
#include "INet.h"
class INet;
class INetMediator
{
public:
	INetMediator(){
		
	}
	virtual ~INetMediator(){}
public:
	//初始化网络
	virtual bool OpenNet() = 0;
	//关闭网络
	virtual bool CloseNet() = 0;
	//发送数据
	virtual bool SendData(char* buf, int nLen, long lSend) = 0;
	//接受数据
	virtual void DealData(char* buf, int nLen, long lSend) = 0;
protected:
	INet* m_pNet;
};