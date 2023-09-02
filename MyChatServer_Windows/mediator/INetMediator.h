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
	//��ʼ������
	virtual bool OpenNet() = 0;
	//�ر�����
	virtual bool CloseNet() = 0;
	//��������
	virtual bool SendData(char* buf, int nLen, long lSend) = 0;
	//��������
	virtual void DealData(char* buf, int nLen, long lSend) = 0;
protected:
	INet* m_pNet;
};