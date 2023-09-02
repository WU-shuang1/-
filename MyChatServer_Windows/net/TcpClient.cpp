#include"TcpClient.h"
#include"TcpClientMediator.h"
#include "packDef.h"
#include <process.h>
TcpClient::TcpClient(INetMediator* mediator):m_clientSock(INVALID_SOCKET), m_handle(NULL), m_isStop(false)
{
	m_pMediator = mediator;
}

TcpClient::~TcpClient()
{
}

//初始化网络
bool TcpClient::InitNet() {
    // 1.加载库
    WORD wVersionRequested;
    WSADATA wsaData;
    int err;

    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
        printf("WSAStartup failed with error: %d\n", err);
        
        return false;
    }

    /* Confirm that the WinSock DLL supports 2.2.*/
    /* Note that if the DLL supports versions greater    */
    /* than 2.2 in addition to 2.2, it will still return */
    /* 2.2 in wVersion since that is the version we      */
    /* requested.                                        */

    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return false;
    }
    else {
        printf("The Winsock 2.2 dll was found okay\n");
    }

    // 2.创建套接字
    m_clientSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_clientSock == INVALID_SOCKET) {
        cout << "sock error: " << WSAGetLastError() << endl;
        WSACleanup();
        return false;
    }
    else {
        cout << "sock success" << endl;
    }

    // 3.连接服务端
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEF_TCP_PORT);
    server_addr.sin_addr.S_un.S_addr = inet_addr("192.168.1.156");

    err = connect(m_clientSock, (sockaddr*)&server_addr, sizeof(server_addr));
    if (err == SOCKET_ERROR) {
        cout << "connect error: " << WSAGetLastError() << endl;
        WSACleanup();
        closesocket(m_clientSock);
        return false;
    }
    else {
        cout << "connect success" << endl;
    }

    //创建接收数据的线程
    m_handle = (HANDLE)_beginthreadex(NULL, 0, &recvDataThread, this, 0, NULL);

    return true;
}

//接收数据
unsigned __stdcall TcpClient::recvDataThread(void* lpVoid) {
    ((TcpClient*)lpVoid)->RecvData();
    return 0;
}
//关闭网络
bool TcpClient::UninitNet() {
	return true;
}
//发送数据
bool TcpClient::SendData(char* buf, int nLen, long lSend) {
    cout << "TcpClient::" << __func__ << endl;
    //先发包大小
    if (send(m_clientSock, (char*)&nLen, sizeof(int), 0) == SOCKET_ERROR) {
        cout << "send len error" << endl;
        return false;
    }
    //再发包内容
    if (send(m_clientSock, buf, nLen, 0) == SOCKET_ERROR) {
        cout << "send buf error" << endl;
        return false;
    }

    return true;
}
//接受数据
void TcpClient::RecvData() {
    cout << "TcpClient::" << __func__ << endl;
    //先接收包大小，再接收包内容
    int nSize;
    int nSizeNum;
    int nOffsetSize;

    int nBufNum;
    int nOffsetBuf;
    while (!m_isStop) {
        //先接包大小
        nSize = 0;
        nSizeNum = 0;
        nOffsetSize = 0;
        while (nOffsetSize < sizeof(int)) {
            nSizeNum = recv(m_clientSock, (char*)&nSize + nOffsetSize, sizeof(int), 0);
            if (SOCKET_ERROR == nSizeNum) {
                cout << "recv len error:" << WSAGetLastError() << endl;
                return;
            }
            nOffsetSize += nSizeNum;
        }
        //再接包内容
        char* buf = new char[nSize];
        nBufNum = 0;
        nOffsetBuf = 0;
        while (nOffsetBuf < nSize) {
            nBufNum = recv(m_clientSock, buf + nOffsetBuf, nSize - nOffsetBuf, 0);
            if (SOCKET_ERROR == nSizeNum) {
                cout << "recv len error:" << WSAGetLastError() << endl;
                return;
            }
            nOffsetBuf += nBufNum;
        }
        //发给中介者
        m_pMediator->DealData(buf, nSize, m_clientSock);
    }
    
}