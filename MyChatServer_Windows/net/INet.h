#pragma once
#include <winsock2.h>
#include <iostream>
using namespace std;

//导入依赖库
#pragma comment(lib, "ws2_32.lib")

class INetMediator;
class INet
{
public:
	INet(){
		
	}
	virtual ~INet(){}

public:
	//初始化网络
	virtual bool InitNet() = 0;
	//关闭网络
	virtual bool UninitNet() = 0;
	//发送数据
	virtual bool SendData(char* buf, int nLen, long lSend) = 0;
protected:
	//接受数据(类内部调用)
	virtual void RecvData() = 0;
protected:
	INetMediator* m_pMediator;
};