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

//初始化网络
bool TcpServerMediator::OpenNet() {
	return m_pNet->InitNet();
}
//关闭网络
bool TcpServerMediator::CloseNet() {
	return m_pNet->UninitNet();
}
//发送数据
bool TcpServerMediator::SendData(char* buf, int nLen, long lSend) {
	return m_pNet->SendData(buf, nLen, lSend);
}
//接受数据
void TcpServerMediator::DealData(char* buf, int nLen, long lSend) {
	//将接收到的数据发送给Kernel
	m_pKernel->dealData(buf, nLen, lSend);
}