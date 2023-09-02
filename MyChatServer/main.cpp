#include <iostream>
#include "CKernel.h"
using namespace std;

int main()
{
	CKernel* pKernel = CKernel::GetKernel();
	if (!pKernel->startServer()) {
		cout << "server open error" << endl;
		return 1;
	}
	//TcpClientMediator mediator;
	//if (!mediator.OpenNet()) {
	//	cout << "client open error" << endl;
	//	return 1;
	//}
	//mediator.SendData((char*)"hello world", sizeof("hello world"),0);


	while (1) {
		cout << "server running..." << endl;
        sleep(3);
	}

	return 0;
}
