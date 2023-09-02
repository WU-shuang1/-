#include"TcpServerMediator.h"
#include"TcpServer.h"
#include "CKernel.h"
TcpServerMediator::TcpServerMediator()
{
	m_pNet = new TcpServer(this);
	m_pKernel = CKernel::GetKernel();
}

TcpServerMediator::~TcpServerMediator()
{
}

//��ʼ������
bool TcpServerMediator::OpenNet() {
	return m_pNet->InitNet();
}
//�ر�����
bool TcpServerMediator::CloseNet() {
	return m_pNet->UninitNet();
}
//��������
bool TcpServerMediator::SendData(char* buf, int nLen, long lSend) {
	return m_pNet->SendData(buf, nLen, lSend);
}
//��������
void TcpServerMediator::DealData(char* buf, int nLen, long lSend) {
	//�����յ������ݷ��͸�Kernel
	m_pKernel->dealData(buf, nLen, lSend);
}