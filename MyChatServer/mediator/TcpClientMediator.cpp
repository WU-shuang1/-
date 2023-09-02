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

//初始化网络
bool TcpClientMediator::OpenNet() {
	return m_pNet->InitNet();
}
//关闭网络
bool TcpClientMediator::CloseNet() {
	return m_pNet->UninitNet();
}
//发送数据
bool TcpClientMediator::SendData(char* buf, int nLen, long lSend) {
	return m_pNet->SendData(buf, nLen, lSend);
}
//接受数据
void TcpClientMediator::DealData(char* buf, int nLen, long lSend) {
	//TODO:将接收到的数据发送给Kernel
	//(CKernel::GetKernel())->dealData(buf, nLen, lSend);
	cout << buf << endl;
}