#pragma once
#include <Windows.h>
#include<map>
using namespace std;

typedef struct {
    CRITICAL_SECTION lock;
    CONDITION_VARIABLE readCondVar;
    CONDITION_VARIABLE writeCondVar;
    int readers;
    int writers;
    int activeWriters;
} ReadWriteLock;
class CLock
{
public:
	CLock();
	~CLock();
public:
    //读写锁
    void initReadWriteLock();
    void readLock();
    void readUnlock();
    void writeLock();
    void writeUnlock();

    //互斥锁
    void Lock();
    void UnLock();
private:
    CRITICAL_SECTION m_lock;    //互斥锁
    CONDITION_VARIABLE m_condVar;       //互斥锁的条件变量
    bool m_flag;                //使用互斥锁的标志

    CRITICAL_SECTION m_rwLock;  //读写锁
    CONDITION_VARIABLE m_readCondVar;   //读锁的条件变量
    CONDITION_VARIABLE m_writeCondVar;  //写锁的条件变量
    int m_readers;  //计数读锁
    int m_writers;  //计数写锁
    int m_activeWriters;    //计数正在写的个数
};







