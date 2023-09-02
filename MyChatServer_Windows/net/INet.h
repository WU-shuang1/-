#pragma once
#include <winsock2.h>
#include <iostream>
using namespace std;

//����������
#pragma comment(lib, "ws2_32.lib")

class INetMediator;
class INet
{
public:
	INet(){
		
	}
	virtual ~INet(){}

public:
	//��ʼ������
	virtual bool InitNet() = 0;
	//�ر�����
	virtual bool UninitNet() = 0;
	//��������
	virtual bool SendData(char* buf, int nLen, long lSend) = 0;
protected:
	//��������(���ڲ�����)
	virtual void RecvData() = 0;
protected:
	INetMediator* m_pMediator;
};