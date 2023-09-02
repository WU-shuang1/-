#include"TcpServer.h"
#include "TcpServerMediator.h"
#include "packDef.h"
#include <process.h>

map<unsigned int, SOCKET> TcpServer::m_mapThreadIdToSocket;	//�洢�̶߳�Ӧ��sock
CLock TcpServer::m_threadLock;		//�������Ͷ�д��

TcpServer::TcpServer(INetMediator* mediator):m_isStop(false)
{
	m_pMediator = mediator;
}

TcpServer::~TcpServer()
{
}

//��ʼ������
bool TcpServer::InitNet() {
    // 1.���ؿ�
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

    // 2.�����׽���
    m_serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_serverSock == INVALID_SOCKET) {
        cout << "sock error: " << WSAGetLastError() << endl;
        WSACleanup();
        return false;
    }
    else {
        cout << "sock success" << endl;
    }

    // 3.��IP��ַ
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEF_TCP_PORT);
    server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
    err = bind(m_serverSock, (sockaddr*)&server_addr, sizeof(server_addr));
    if (err == SOCKET_ERROR) {
        cout << "bind error: " << WSAGetLastError() << endl;
        closesocket(m_serverSock);
        WSACleanup();
        return false;
    }
    else {
        cout << "bind success" << endl;
    }
    // 4.����
    err = listen(m_serverSock, 100);
    if (SOCKET_ERROR == err) {
        cout << "listen error: " << WSAGetLastError() << endl;
        closesocket(m_serverSock);
        WSACleanup();
        return false;
    }

    //�������������߳�
    HANDLE handle = (HANDLE)_beginthreadex(0, 0, &acceptClientThread, this, 0, NULL);
    if (handle == nullptr) {
        cout << "acceptClientThread Creat error: " << GetLastError() << endl;
    }
    m_lstHandle.push_back(handle);
	return true;
}
//��������
unsigned __stdcall TcpServer::acceptClientThread(void* lpVoid) {
    TcpServer* pthis = (TcpServer*)lpVoid;
    unsigned int threadId = 0;
    while (!pthis->m_isStop) {
        SOCKET sock = accept(pthis->m_serverSock, nullptr, nullptr);
        if (sock == INVALID_SOCKET) {
            cout << "accept failed: " << WSAGetLastError() << endl;
            return 1;
        }
        //�������ոÿͻ������ݵ��̣߳���ʼΪ����״̬�� ��ֹmap��û�н��б���ͽ������ݽ���
        HANDLE handle = (HANDLE)_beginthreadex(0, 0, &recvDataThread, pthis, CREATE_SUSPENDED, &threadId);
        if (handle == INVALID_HANDLE_VALUE) {
            //����ʧ��
            cout << "recvDataThread Creat error: " << GetLastError() << endl;
        }
        pthis->m_lstHandle.push_back(handle);   //���������д洢

        //extern HANDLE m_mapThreadIdToSocketMutex;
        //DWORD dwWaitResult = WaitForSingleObject(m_mapThreadIdToSocketMutex, INFINITE);
        //if (dwWaitResult == WAIT_OBJECT_0) {
        //    // �������ѻ�ȡ��ִ�й�����Դ���ʲ���
        m_threadLock.Lock();
        m_mapThreadIdToSocket[threadId] = sock;
        m_threadLock.UnLock();
        //    // �ͷ���
        //    ReleaseMutex(m_mapThreadIdToSocketMutex);
        //}
        //pthis->m_mapThreadIdToSocket[threadId] = sock;    //��socket������map

        //�����߳�
        DWORD dword = ResumeThread(handle);
        if (dword == -1 || dword == 0) {
            cout << "ResumeThread error " << GetLastError() << endl;
            return 1;
        }
    }
}
//��������
unsigned __stdcall TcpServer::recvDataThread(void* lpVoid) {
    
    ((TcpServer*)lpVoid)->RecvData();
    return 0;
}

//�ر����磺������Դ �����߳� �ر��׽��� ж�ؿ�
bool TcpServer::UninitNet() {
    //�л�����״̬
    m_isStop = true;
    //�����߳�
    auto lstite = m_lstHandle.begin();
    while (lstite != m_lstHandle.end()) {
        if (*lstite) {
            if (WAIT_TIMEOUT == WaitForSingleObject(*lstite, 100)) {
                //100ms��δ�ر�
                TerminateThread(*lstite, -1);
            }
            //���پ�����ں˶������ü�����-1������0ʱ��ϵͳ�Զ������ں˶���
            delete* lstite;
            *lstite = NULL;
            lstite = m_lstHandle.erase(lstite);
        }
        else lstite++;
    }
    m_lstHandle.clear();

    //�ر����пͻ��˵��׽���
    auto mapite = m_mapThreadIdToSocket.begin();
    while (mapite != m_mapThreadIdToSocket.end()) {
        if (mapite->second && mapite->second != INVALID_SOCKET) {
            closesocket(mapite->second);
            mapite = m_mapThreadIdToSocket.erase(mapite);
        }
        else mapite++;
    }
    m_mapThreadIdToSocket.clear();
    //�رշ���˵��׽���
    if (m_serverSock && m_serverSock != INVALID_SOCKET) {
        closesocket(m_serverSock);
    }
    //ж�ؿ�
    WSACleanup();
	return true;
}
//��������
bool TcpServer::SendData(char* buf, int nLen, long lSend) {
    cout << "TcpServer::" << __func__ << endl;
    //�ȷ�����С
    if (send(lSend, (char*)&nLen, sizeof(int), 0) == SOCKET_ERROR) {
        cout << "send len error" <<WSAGetLastError()<< endl;
        return false;
    }
    //�ٷ�������
    if (send(lSend, buf, nLen, 0) == SOCKET_ERROR) {
        cout << "send buf error" << WSAGetLastError() << endl;
        return false;
    }

	return true;
}
//��������
void TcpServer::RecvData() {
    cout << "TcpServer::" << __func__ << endl;
    //��ȡ��ǰ�̵߳�socket
    m_threadLock.Lock();
    SOCKET sock = m_mapThreadIdToSocket[GetCurrentThreadId()];
    m_threadLock.UnLock();

    int nSize;
    int nSizeNum;
    int nOffsetSize;

    int nBufNum;
    int nOffsetBuf;
    while (!m_isStop) {
        //�ȽӰ���С
        nSize = 0;
        nSizeNum = 0;
        nOffsetSize = 0;
        while (nOffsetSize < sizeof(int)) {
            nSizeNum = recv(sock, (char*)&nSize + nOffsetSize, sizeof(int), 0);
            if (SOCKET_ERROR == nSizeNum) {
                cout << "recv len error:" << WSAGetLastError() << endl;
                if (WSAGetLastError() == 10038 || WSAGetLastError() == 10054) {
                    m_threadLock.Lock();
                    closesocket(m_mapThreadIdToSocket[GetCurrentThreadId()]);
                    m_mapThreadIdToSocket.erase(GetCurrentThreadId());
                    m_threadLock.UnLock();
                }
                return;
            }
            nOffsetSize += nSizeNum;
        }
        //�ٽӰ�����
        char* buf = new char[nSize];
        nBufNum = 0;
        nOffsetBuf = 0;
        while (nOffsetBuf < nSize) {
            nBufNum = recv(sock, buf + nOffsetBuf, nSize - nOffsetBuf, 0);
            if (SOCKET_ERROR == nSizeNum) {
                cout << "recv len error:" << WSAGetLastError() << endl;
                if (WSAGetLastError() == 10038 || WSAGetLastError() == 10054) {
                    m_threadLock.Lock();
                    closesocket(m_mapThreadIdToSocket[GetCurrentThreadId()]);
                    m_mapThreadIdToSocket.erase(GetCurrentThreadId());
                    m_threadLock.UnLock();
                }
                return;
            }
            nOffsetBuf += nBufNum;
        }
        //�����н���
        m_pMediator->DealData(buf, nSize, sock);
    }
}