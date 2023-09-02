#pragma once
#include"INetMediator.h"

class TcpClientMediator :public INetMediator
{
public:
	TcpClientMediator();
	~TcpClientMediator();

public:
	//��ʼ������
	virtual bool OpenNet();
	//�ر�����
	virtual bool CloseNet();
	//��������
	virtual bool SendData(char* buf, int nLen, long lSend);
	//��������
	virtual void DealData(char* buf, int nLen, long lSend);
};

