#pragma once
#include"INetMediator.h"
class CKernel;
class TcpServerMediator :public INetMediator
{
public:
	TcpServerMediator();
	~TcpServerMediator();

public:
	//��ʼ������
	virtual bool OpenNet();
	//�ر�����
	virtual bool CloseNet();
	//��������
	virtual bool SendData(char* buf, int nLen, long lSend);
	//��������
	virtual void DealData(char* buf, int nLen, long lSend);
private:
	CKernel* m_pKernel;
};