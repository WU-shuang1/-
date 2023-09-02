#include"TcpClientMediator.h"
#include"TcpClient.h"
#include"CKernel.h"
TcpClientMediator::TcpClientMediator()
{
	m_pNet = new TcpClient(this);
}

TcpClientMediator::~TcpClientMediator()
{
}

//��ʼ������
bool TcpClientMediator::OpenNet() {
	return m_pNet->InitNet();
}
//�ر�����
bool TcpClientMediator::CloseNet() {
	return m_pNet->UninitNet();
}
//��������
bool TcpClientMediator::SendData(char* buf, int nLen, long lSend) {
	return m_pNet->SendData(buf, nLen, lSend);
}
//��������
void TcpClientMediator::DealData(char* buf, int nLen, long lSend) {
	//TODO:�����յ������ݷ��͸�Kernel
	//(CKernel::GetKernel())->dealData(buf, nLen, lSend);
	cout << buf << endl;
}