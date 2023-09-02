#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>
#include <error.h>
#define TIMEOUT 1
struct bs_t{
    void* (*business)(void*);
	void* arg;
};

class CThreadPool {
	public:
		CThreadPool(){}
		~CThreadPool(){}
	public:
		int thread_pool_create(int Max, int Min, int Qmax);	//线程池创建初始化
		int Producer_add_task(bs_t bs);	//生产者添加任务模块
		static void* Customer_thread(void* arg);	//消费者线程，参数为线程池地址
		static void* Manager_thread(void* arg);	//管理者线程，参数为线程池地址
		int thread_pool_destroy();	//销毁线程
		bool isThreadAlive(pthread_t tid);	//检测线程是否存活
	private:
		int m_thread_shutdown;	//线程池开关，如果为1，关闭线程池
		int m_thread_max;	//最大线程数
		int m_thread_min;	//最小线程数
		int m_thread_alive;	//有效线程数
		int m_thread_busy;	//繁忙线程
		int m_kill_number;	//缩减码

		bs_t* m_queue;	//任务队列
		int m_front;
		int m_rear;
		int m_cur;
		int m_max;
		pthread_cond_t m_Not_Full;
		pthread_cond_t m_Not_Empty;
		pthread_t* m_ctids;	//存储消费者id
		pthread_t m_mtid;	//存储管理器
		pthread_mutex_t m_lock;
};	//线程池

void* user_business(void* arg);
