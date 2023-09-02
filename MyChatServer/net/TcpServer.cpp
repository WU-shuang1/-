#include"TcpServer.h"
#include "TcpServerMediator.h"
#include "packDef.h"
//#include <process.h>

//map<unsigned int, SOCKET> TcpServer::m_mapThreadIdToSocket;	//存储线程对应的sock
//CLock TcpServer::m_threadLock;		//互斥锁和读写锁

TcpServer::TcpServer(INetMediator* mediator):m_isStop(false)
{
	m_pMediator = mediator;
}

TcpServer::~TcpServer()
{
}

//初始化网络
bool TcpServer::InitNet() {
//    // 1.加载库
//    WORD wVersionRequested;
//    WSADATA wsaData;
//    int err;

//    /* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
//    wVersionRequested = MAKEWORD(2, 2);

//    err = WSAStartup(wVersionRequested, &wsaData);
//    if (err != 0) {
//        /* Tell the user that we could not find a usable */
//        /* Winsock DLL.                                  */
//        printf("WSAStartup failed with error: %d\n", err);
//        return false;
//    }

//    /* Confirm that the WinSock DLL supports 2.2.*/
//    /* Note that if the DLL supports versions greater    */
//    /* than 2.2 in addition to 2.2, it will still return */
//    /* 2.2 in wVersion since that is the version we      */
//    /* requested.                                        */

//    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
//        /* Tell the user that we could not find a usable */
//        /* WinSock DLL.                                  */
//        printf("Could not find a usable version of Winsock.dll\n");
//        WSACleanup();
//        return false;
//    }
//    else {
//        printf("The Winsock 2.2 dll was found okay\n");
//    }

//    // 2.创建套接字
//    m_serverSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//    if (m_serverSock == INVALID_SOCKET) {
//        cout << "sock error: " << WSAGetLastError() << endl;
//        WSACleanup();
//        return false;
//    }
//    else {
//        cout << "sock success" << endl;
//    }

//    // 3.绑定IP地址
//    sockaddr_in server_addr;
//    server_addr.sin_family = AF_INET;
//    server_addr.sin_port = htons(DEF_TCP_PORT);
//    server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
//    err = bind(m_serverSock, (sockaddr*)&server_addr, sizeof(server_addr));
//    if (err == SOCKET_ERROR) {
//        cout << "bind error: " << WSAGetLastError() << endl;
//        closesocket(m_serverSock);
//        WSACleanup();
//        return false;
//    }
//    else {
//        cout << "bind success" << endl;
//    }
//    // 4.监听
//    err = listen(m_serverSock, 100);
//    if (SOCKET_ERROR == err) {
//        cout << "listen error: " << WSAGetLastError() << endl;
//        closesocket(m_serverSock);
//        WSACleanup();
//        return false;
//    }

//    //创建接受连接线程
//    HANDLE handle = (HANDLE)_beginthreadex(0, 0, &acceptClientThread, this, 0, NULL);
//    if (handle == nullptr) {
//        cout << "acceptClientThread Creat error: " << GetLastError() << endl;
//    }
//    m_lstHandle.push_back(handle);
//	return true;
    m_serverSock = socket(AF_INET, SOCK_STREAM, 0);

    //set REUSERADDR
    int flags = 1;
    int ret = setsockopt(m_serverSock, SOL_SOCKET, SO_REUSEADDR, (char *)&flags, sizeof(flags));
    if ( ret == -1 ){
        perror("setsockopt error");
        return false;
    }

    struct sockaddr_in addr;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(DEF_TCP_PORT);
    addr.sin_family = AF_INET;

    if(bind(m_serverSock, (struct sockaddr*)&addr, sizeof(addr))) {
        perror("Bind failed");
        return false;
    }

    if(listen(m_serverSock, DEF_TCP_BACKLOG) == -1) {
        perror("Listen failed");
        return false;
    }

    m_threadPool = new CThreadPool;
    m_threadPool->thread_pool_create(300,10,10000);


    m_epfd = epoll_create(DEF_EPOLL_MAX);

    struct epoll_event node;
    node.data.fd = m_serverSock;
    node.events = EPOLLIN;
    epoll_ctl(m_epfd, EPOLL_CTL_ADD, m_serverSock, &node);

    EpollListen();
}
//接受连接
//void* TcpServer::acceptClientThread(void* lpVoid) {
//    TcpServer* pthis = (TcpServer*)lpVoid;
//    //unsigned int threadId = 0;
//    while (!pthis->m_isStop) {
//        int sock = accept(pthis->m_serverSock, nullptr, nullptr);
//        if (sock == -1) {
//            cout << "accept failed: " << endl;
//            return nullptr;
//        }
        //创建接收该客户端数据的线程（初始为挂起状态） 防止map还没有进行保存就进行数据接收
        //HANDLE handle = (HANDLE)_beginthreadex(0, 0, &recvDataThread, pthis, CREATE_SUSPENDED, &threadId);
//        if (handle == INVALID_HANDLE_VALUE) {
//            //创建失败
//            cout << "recvDataThread Creat error: " << GetLastError() << endl;
//        }
        //pthis->m_lstHandle.push_back(handle);   //放入链表中存储

        //extern HANDLE m_mapThreadIdToSocketMutex;
        //DWORD dwWaitResult = WaitForSingleObject(m_mapThreadIdToSocketMutex, INFINITE);
        //if (dwWaitResult == WAIT_OBJECT_0) {
        //    // 互斥锁已获取，执行共享资源访问操作
//        m_threadLock.Lock();
//        m_mapThreadIdToSocket[threadId] = sock;
//        m_threadLock.UnLock();
        //    // 释放锁
        //    ReleaseMutex(m_mapThreadIdToSocketMutex);
        //}
        //pthis->m_mapThreadIdToSocket[threadId] = sock;    //将socket保存至map

        //唤醒线程
//        DWORD dword = ResumeThread(handle);
//        if (dword == -1 || dword == 0) {
//            cout << "ResumeThread error " << GetLastError() << endl;
//            return 1;
//        }
//    }
//}
//接收数据
//void* TcpServer::recvDataThread(void* lpVoid) {
    
//    ((TcpServer*)lpVoid)->RecvData();
//    return 0;
//}

//关闭网络：回收资源 回收线程 关闭套接字 卸载库
bool TcpServer::UninitNet() {
    //切换运行状态
    m_isStop = true;
//    //回收线程
//    auto lstite = m_lstHandle.begin();
//    while (lstite != m_lstHandle.end()) {
//        if (*lstite) {
//            if (WAIT_TIMEOUT == WaitForSingleObject(*lstite, 100)) {
//                //100ms后还未关闭
//                TerminateThread(*lstite, -1);
//            }
//            //销毁句柄，内核对象引用计数器-1，减到0时，系统自动回收内核对象
//            delete* lstite;
//            *lstite = NULL;
//            lstite = m_lstHandle.erase(lstite);
//        }
//        else lstite++;
//    }
//    m_lstHandle.clear();

//    //关闭所有客户端的套接字
//    auto mapite = m_mapThreadIdToSocket.begin();
//    while (mapite != m_mapThreadIdToSocket.end()) {
//        if (mapite->second && mapite->second != INVALID_SOCKET) {
//            closesocket(mapite->second);
//            mapite = m_mapThreadIdToSocket.erase(mapite);
//        }
//        else mapite++;
//    }
//    m_mapThreadIdToSocket.clear();
//    //关闭服务端的套接字
//    if (m_serverSock && m_serverSock != INVALID_SOCKET) {
//        closesocket(m_serverSock);
//    }
//    //卸载库
//    WSACleanup();
	return true;
}
//发送数据
bool TcpServer::SendData(char* buf, int nLen, long lSend) {
    cout << "TcpServer::" << __func__ << endl;
    //先发包大小
    if (send(lSend, (char*)&nLen, sizeof(int), 0) < 0) {
        perror("send len error");
        //cout << "send len error" <<WSAGetLastError()<< endl;
        return false;
    }
    //再发包内容
    if (send(lSend, buf, nLen, 0) < 0) {
        perror("send buf error");
        //cout << "send buf error" << WSAGetLastError() << endl;
        return false;
    }
	return true;
}
//接受数据
void TcpServer::RecvData() {
//    cout << "TcpServer::" << __func__ << endl;
//    //提取当前线程的socket
//    m_threadLock.Lock();
//    //SOCKET sock = m_mapThreadIdToSocket[GetCurrentThreadId()];
//    m_threadLock.UnLock();

//    int nSize;
//    int nSizeNum;
//    int nOffsetSize;

//    int nBufNum;
//    int nOffsetBuf;
//    while (!m_isStop) {
//        //先接包大小
//        nSize = 0;
//        nSizeNum = 0;
//        nOffsetSize = 0;
//        while (nOffsetSize < sizeof(int)) {
//            nSizeNum = recv(sock, (char*)&nSize + nOffsetSize, sizeof(int), 0);
//            if (SOCKET_ERROR == nSizeNum) {
//                cout << "recv len error:" << WSAGetLastError() << endl;
//                if (WSAGetLastError() == 10038 || WSAGetLastError() == 10054) {
//                    m_threadLock.Lock();
//                    closesocket(m_mapThreadIdToSocket[GetCurrentThreadId()]);
//                    m_mapThreadIdToSocket.erase(GetCurrentThreadId());
//                    m_threadLock.UnLock();
//                }
//                return;
//            }
//            nOffsetSize += nSizeNum;
//        }
//        //再接包内容
//        char* buf = new char[nSize];
//        nBufNum = 0;
//        nOffsetBuf = 0;
//        while (nOffsetBuf < nSize) {
//            nBufNum = recv(sock, buf + nOffsetBuf, nSize - nOffsetBuf, 0);
//            if (SOCKET_ERROR == nSizeNum) {
//                cout << "recv len error:" << WSAGetLastError() << endl;
//                if (WSAGetLastError() == 10038 || WSAGetLastError() == 10054) {
//                    m_threadLock.Lock();
//                    closesocket(m_mapThreadIdToSocket[GetCurrentThreadId()]);
//                    m_mapThreadIdToSocket.erase(GetCurrentThreadId());
//                    m_threadLock.UnLock();
//                }
//                return;
//            }
//            nOffsetBuf += nBufNum;
//        }
//        //发给中介者
//        m_pMediator->DealData(buf, nSize, sock);
//    }
}

void TcpServer::EpollListen()
{
    struct sockaddr_in addr;
    struct epoll_event node;
    unsigned int addrSize = sizeof(addr);
    int readyCode;
    int clientSock;
    int i;
    while (!m_isStop) {
        readyCode = epoll_wait(m_epfd, m_events, DEF_EPOLL_MAX+1, 1000);
        i = 0;
        while(readyCode>0) {
            if(m_events[i].data.fd == m_serverSock) {
                if((clientSock = accept(m_serverSock, (struct sockaddr*)&addr, &addrSize)) == -1) {
                    perror("accept failed");
                    exit(0);
                }
                int value = 1;
                setsockopt(clientSock, IPPROTO_TCP, TCP_NODELAY ,(char *)&value, sizeof(int));

                cout<<"accept"<<endl;
                node.data.fd = clientSock;
                node.events = EPOLLIN|EPOLLONESHOT;
                epoll_ctl(m_epfd, EPOLL_CTL_ADD, clientSock, &node);

            }
            else {
                bs_t bs;
                bs.business = &TcpServer::recvDataTask;
                Node* taskArg = new Node;
                taskArg->fd = m_events[i].data.fd;
                taskArg->ptr = this;
                bs.arg = taskArg;
                m_threadPool->Producer_add_task(bs);
            }
            i++;
            readyCode--;
        }
    }
}

void *TcpServer::recvDataTask(void *lpVoid)
{
    cout<<__func__<<endl;
    Node* node = (Node*)lpVoid;
    TcpServer* pthis = (TcpServer*)node->ptr;
    int sockfd = node->fd;

    delete node;
    node = nullptr;

    int bufSize = 0;
    //先接包大小
    do {
        int nRelReadNum = read(sockfd,&bufSize,sizeof(bufSize));
        if(nRelReadNum != 4) {
            break;
        }
        //cout << "bufSize: "<<bufSize<<" nOffsetSize:"<<nOffsetSize<<endl;
        int nBufNum = 0;
        int offPos = 0;
        char* buf = new char[bufSize];
        while(bufSize) {
            nBufNum = recv(sockfd,buf + offPos, bufSize, 0);
            if(nBufNum <= 0) {
                break;
            }
            offPos += nBufNum;
            bufSize -= nBufNum;
        }
        cout << "bufSize: "<<bufSize<<" offPos:"<<offPos<<endl;
        bs_t bs;
        bs.business = &TcpServer::dealBufTask;
        DataBuffer* Databuf = new DataBuffer(pthis, sockfd, buf, bufSize);
        bs.arg = Databuf;
        pthis->m_threadPool->Producer_add_task(bs);


        struct epoll_event epollNode;
        epollNode.data.fd = sockfd;
        epollNode.events = EPOLLIN | EPOLLONESHOT;
        if(epoll_ctl(pthis->m_epfd, EPOLL_CTL_MOD, sockfd, &epollNode) == -1) {
            perror("epoll_ctl error");
            exit(0);
        }
        return 0;
    }while(0);

    //delete sock
    cout<< "delete sock"<<endl;
    epoll_ctl(pthis->m_epfd, EPOLL_CTL_DEL, sockfd, NULL);
    close(sockfd);

}

void *TcpServer::dealBufTask(void *lpVoid)
{
    cout<<__func__<<endl;
    DataBuffer* buf = (DataBuffer*)lpVoid;
    TcpServer* pthis = (TcpServer*)buf->ptr;
    pthis->m_pMediator->DealData(buf->buf,buf->nlen, buf->sockfd);
    delete buf;
    buf = nullptr;
}
