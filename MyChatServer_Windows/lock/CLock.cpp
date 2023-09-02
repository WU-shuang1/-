#include "CLock.h"

CLock::CLock() {
	//��ʼ����
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
//		// �������ѻ�ȡ��ִ�й�����Դ���ʲ���
//		m_mapIdToSocket.erase(GetCurrentThreadId());
//		// �ͷ���
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
    //��ȡ������
    EnterCriticalSection(&m_rwLock);
    while (m_writers > 0 || m_activeWriters > 0) {
        //��׼��д���̣߳��������߳�����д������
        SleepConditionVariableCS(&m_readCondVar, &m_rwLock, INFINITE);
    }
    m_readers++;
    //�ͷŻ�����
    LeaveCriticalSection(&m_rwLock);
}

void CLock::readUnlock() {
    EnterCriticalSection(&m_rwLock);
    m_readers--;
    if (m_readers == 0 && m_writers > 0) {
        //û���̶߳��������߳��ڵȴ�д������д�߳�
        WakeConditionVariable(&m_writeCondVar);
    }
    //�ͷ���
    LeaveCriticalSection(&m_rwLock);
}

void CLock::writeLock() {
    //��ȡ������
    EnterCriticalSection(&m_rwLock);
    //д�߳�+1
    m_writers++;
    while (m_readers > 0 || m_activeWriters > 0) {
        //�����ڶ����̻߳�����д���̣߳�����
        SleepConditionVariableCS(&m_writeCondVar, &m_rwLock, INFINITE);
    }
    //�����Ѻ�ʼ����д ����д�߳�+1
    m_activeWriters++;
    LeaveCriticalSection(&m_rwLock);
}

void CLock::writeUnlock() {
    //��ȡ��
    EnterCriticalSection(&m_rwLock);
    //д���д�߳�-1��׼��д�߳�-1
    m_activeWriters--;
    m_writers--;
    if (m_writers > 0) {
        //������Ҫд���̣߳�����д�߳�
        WakeConditionVariable(&m_writeCondVar);
    }
    else {
        //û��д�̣߳��������ж��߳�
        WakeAllConditionVariable(&m_readCondVar);
    }
    //�ͷ���
    LeaveCriticalSection(&m_rwLock);
}

void CLock::Lock() {
    //��ȡ��
    EnterCriticalSection(&m_lock);
    while (m_flag) {
        SleepConditionVariableCS(&m_condVar, &m_lock, INFINITE);
    }
    m_flag = true;
    //�ͷ���
    LeaveCriticalSection(&m_lock);
}
void CLock::UnLock() {
    //��ȡ��
    EnterCriticalSection(&m_lock);
    while (!m_flag) {
        SleepConditionVariableCS(&m_condVar, &m_lock, INFINITE);
    }
    m_flag = false;
    //�������������е�һ���߳�
    WakeConditionVariable(&m_condVar);
    //�ͷ���
    LeaveCriticalSection(&m_lock);
}