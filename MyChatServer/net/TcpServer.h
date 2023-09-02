#pragma once
#include"INet.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <errno.h>
#include <signal.h>
#include <list>
#include <map>
#include <thread_pool.h>
#include "packDef.h"

struct Node {
    void* ptr;
    int fd;
};
struct DataBuffer
{
    DataBuffer( void* _ptr,  int _sock , char* _buf , int _nlen )
        :ptr(_ptr),sockfd(_sock),buf(_buf),nlen(_nlen){}

    void* ptr;
    int sockfd;
    char* buf;
    int nlen;

};
class TcpServer :public INet
{
public:
	TcpServer(INetMediator* mediator);
	~TcpServer();

public:
	//初始化网络
	virtual bool InitNet();
	//关闭网络
	virtual bool UninitNet();
	//发送数据
	virtual bool SendData(char* buf, int nLen, long lSend);
public:
    //static map<unsigned int, SOCKET> m_mapThreadIdToSocket;	//存储线程对应的sock
    //static CLock m_threadLock;		//互斥锁和读写锁
private:
	//接受数据
	virtual void RecvData();
    void EpollListen();
    static void* acceptTask(void* lpVoid);
    static void* recvDataTask(void* lpVoid);
    static void* dealBufTask(void* lpVoid);

    CThreadPool* m_threadPool;
    int m_serverSock;
    int m_epfd;
    struct epoll_event m_events[DEF_EPOLL_MAX+1];
    //list<HANDLE> m_lstHandle;	//存储线程句柄
	
	bool m_isStop;
};
