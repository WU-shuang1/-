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
    //��д��
    void initReadWriteLock();
    void readLock();
    void readUnlock();
    void writeLock();
    void writeUnlock();

    //������
    void Lock();
    void UnLock();
private:
    CRITICAL_SECTION m_lock;    //������
    CONDITION_VARIABLE m_condVar;       //����������������
    bool m_flag;                //ʹ�û������ı�־

    CRITICAL_SECTION m_rwLock;  //��д��
    CONDITION_VARIABLE m_readCondVar;   //��������������
    CONDITION_VARIABLE m_writeCondVar;  //д������������
    int m_readers;  //��������
    int m_writers;  //����д��
    int m_activeWriters;    //��������д�ĸ���
};







