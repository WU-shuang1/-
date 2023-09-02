#include "CLock.h"

CLock::CLock() {
	//初始化锁
    initReadWriteLock();
}
CLock::~CLock() {
	
}

//void CLock::idMutex(map<int, SOCKET> &m_mapIdToSocket) {
//
//}
//
//void CLock::threadMutex(map<unsigned int, SOCKET> &m_mapIdToSocket) {
//	DWORD dwWaitResult_thread = WaitForSingleObject(m_mapThreadIdToSocketMutex, INFINITE);
//	if (dwWaitResult_thread == WAIT_OBJECT_0) {
//		// 互斥锁已获取，执行共享资源访问操作
//		m_mapIdToSocket.erase(GetCurrentThreadId());
//		// 释放锁
//		ReleaseMutex(m_mapThreadIdToSocketMutex);
//	}
//}

void  CLock::initReadWriteLock() {
    InitializeCriticalSection(&m_lock);
    InitializeConditionVariable(&m_condVar);
    m_flag = false;

    InitializeCriticalSection(&m_rwLock);
    InitializeConditionVariable(&m_readCondVar);
    InitializeConditionVariable(&m_writeCondVar);
    
    
    m_readers = 0;
    m_writers = 0;
    m_activeWriters = 0;
}
void  CLock::readLock() {
    //获取互斥锁
    EnterCriticalSection(&m_rwLock);
    while (m_writers > 0 || m_activeWriters > 0) {
        //有准备写的线程，或者有线程正在写，挂起
        SleepConditionVariableCS(&m_readCondVar, &m_rwLock, INFINITE);
    }
    m_readers++;
    //释放互斥锁
    LeaveCriticalSection(&m_rwLock);
}

void CLock::readUnlock() {
    EnterCriticalSection(&m_rwLock);
    m_readers--;
    if (m_readers == 0 && m_writers > 0) {
        //没有线程读并且有线程在等待写，唤醒写线程
        WakeConditionVariable(&m_writeCondVar);
    }
    //释放锁
    LeaveCriticalSection(&m_rwLock);
}

void CLock::writeLock() {
    //获取互斥锁
    EnterCriticalSection(&m_rwLock);
    //写线程+1
    m_writers++;
    while (m_readers > 0 || m_activeWriters > 0) {
        //有正在读的线程或正在写的线程，挂起
        SleepConditionVariableCS(&m_writeCondVar, &m_rwLock, INFINITE);
    }
    //被唤醒后开始进行写 正在写线程+1
    m_activeWriters++;
    LeaveCriticalSection(&m_rwLock);
}

void CLock::writeUnlock() {
    //获取锁
    EnterCriticalSection(&m_rwLock);
    //写完后写线程-1，准备写线程-1
    m_activeWriters--;
    m_writers--;
    if (m_writers > 0) {
        //后面有要写的线程，唤醒写线程
        WakeConditionVariable(&m_writeCondVar);
    }
    else {
        //没有写线程，唤醒所有读线程
        WakeAllConditionVariable(&m_readCondVar);
    }
    //释放锁
    LeaveCriticalSection(&m_rwLock);
}

void CLock::Lock() {
    //获取锁
    EnterCriticalSection(&m_lock);
    while (m_flag) {
        SleepConditionVariableCS(&m_condVar, &m_lock, INFINITE);
    }
    m_flag = true;
    //释放锁
    LeaveCriticalSection(&m_lock);
}
void CLock::UnLock() {
    //获取锁
    EnterCriticalSection(&m_lock);
    while (!m_flag) {
        SleepConditionVariableCS(&m_condVar, &m_lock, INFINITE);
    }
    m_flag = false;
    //唤醒条件变量中的一个线程
    WakeConditionVariable(&m_condVar);
    //释放锁
    LeaveCriticalSection(&m_lock);
}