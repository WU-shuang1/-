#include <cerrno>
#include <thread_pool.h>

int CThreadPool::thread_pool_create(int Max, int Min, int Qmax) {
	//初始化成员属性
	m_thread_max = Max;
	m_thread_min = Min;
	m_thread_shutdown = 1;
	m_thread_alive = 0;
	m_thread_busy = 0;
	m_kill_number = 0;

	if((m_queue = new bs_t[Qmax]) == nullptr) {
		perror("thread_pool_create new queue Failed");
		exit(0);
	}

    if((m_ctids = new pthread_t[Max]{0}) == nullptr) {
		perror("thread_pool_create new ctids Failed");
		exit(0);
	}

    //bzero(m_ctids, sizeof(pthread_t) * Max);
	bzero(m_queue, sizeof(bs_t)* Qmax);

	m_front = 0;
	m_rear = 0;
	m_cur = 0;
	m_max = Qmax;

	if(pthread_mutex_init(&m_lock, NULL) != 0) {
		printf("thread_pool_create init Lock Failed");
		exit(0);
	}
	if(pthread_cond_init(&m_Not_Full, NULL) != 0 || pthread_cond_init(&m_Not_Empty, NULL) != 0) {
		printf("thread_pool_create init Cond Failed");
		exit(0);
	}
    int err;
    //创建管理者线程
    if((err = pthread_create(&m_mtid, NULL, Manager_thread, (void*)this)) > 0) {
        printf("thread_pool_create create manager thread failed:%s\n",strerror(err));
        exit(0);
    }

	//创建消费者线程

	for(int i=0; i<Min; i++) {
		if((err = pthread_create(&m_ctids[i], NULL, Customer_thread, (void*)this)) > 0) {
			printf("thread_pool_create create customer thread failed:%s\n",strerror(err));
			exit(0);
		}
		m_thread_alive++;
	}


	return 0;
}
int CThreadPool::Producer_add_task(bs_t bs){
	//生产者向任务队列中添加一次任务
	if(m_thread_shutdown) {
		pthread_mutex_lock(&m_lock);
		while(m_cur == m_max) {
			//任务队列满载，挂起等待
			pthread_cond_wait(&m_Not_Full, &m_lock);
			if(!m_thread_shutdown) {
				pthread_mutex_unlock(&m_lock);
				printf("thread shutdown 0, Main thread exit..\n");
				pthread_exit(NULL);
			}
		}
		//添加任务
        printf("pool info: Max %d Min %d Alive %d Busy %d idel %d Busy/Alive %.2f Alive/Max %.2f\n",m_thread_max,m_thread_min,m_thread_alive,m_thread_busy,
               m_thread_alive-m_thread_busy,(double)m_thread_busy/m_thread_alive,(double)m_thread_alive/m_thread_max);
		m_queue[m_front].business = bs.business;
		m_queue[m_front].arg = bs.arg;
		m_front = (m_front + 1) % m_max;
        m_cur++;

		//唤醒消费者
		pthread_cond_signal(&m_Not_Empty);
		pthread_mutex_unlock(&m_lock);
	}else {
		printf("thread shutdown 0, Main thread exit..\n");
		pthread_exit(NULL);

	}
	printf("Producer thread [0x%x] add Business success, business_addr = %p\n",(unsigned int)pthread_self(), bs.business);

	return 0;
}
void* CThreadPool::Customer_thread(void* arg){

	CThreadPool* pthis = (CThreadPool*)arg;
	bs_t bs;
	pthread_detach(pthread_self());
	while(pthis->m_thread_shutdown) {
		//持续尝试获取任务
		pthread_mutex_lock(&pthis->m_lock);
		while(pthis->m_cur == 0) {
			pthread_cond_wait(&pthis->m_Not_Empty, &pthis->m_lock);
			if(!pthis->m_thread_shutdown) {
				pthread_mutex_unlock(&pthis->m_lock);
				printf("Customer thread [0x%x] shutdowm its 0, exiting..\n", (unsigned int) pthread_self());
				pthread_exit(NULL);
			}
			if(pthis->m_kill_number) {
				--pthis->m_thread_alive;
				--pthis->m_kill_number;
				printf("Customer thread [0x%x] exit, kill_number %d..\n",(unsigned int)pthread_self(), pthis->m_kill_number);
				pthread_mutex_unlock(&pthis->m_lock);
				pthread_exit(NULL);
			}
            //printf("dasdasd\n");
		}
		//获取任务，执行任务
		bs.business = pthis->m_queue[pthis->m_rear].business;
		bs.arg = pthis->m_queue[pthis->m_rear].arg;
		--(pthis->m_cur);
		pthis->m_rear = (pthis->m_rear + 1) % pthis->m_max;
		pthis->m_thread_busy++;
		//唤醒生产者
		pthread_cond_signal(&pthis->m_Not_Full);
		pthread_mutex_unlock(&pthis->m_lock);
		printf("Customer thread[0x%x] Getbusiness_addr %p, Runing..\n", (unsigned int)pthread_self(), &bs.business);
        //执行任务
		(*bs.business)(bs.arg);
		//忙线程--
		pthread_mutex_lock(&pthis->m_lock);       
		--pthis->m_thread_busy;
		pthread_mutex_unlock(&pthis->m_lock);
	}

	return nullptr;
}
void* CThreadPool::Manager_thread(void* arg){
	pthread_detach(pthread_self());
	CThreadPool* pthis = (CThreadPool*)arg;
	int alive, cur, busy;
	pthread_mutex_lock(&pthis->m_lock);
	alive = pthis->m_thread_alive;
	cur = pthis->m_cur;
	busy = pthis->m_thread_busy;
	pthread_mutex_unlock(&pthis->m_lock);

	int flag,add;
	//持续执行
	while(pthis->m_thread_shutdown) {

		if((cur >= alive - busy || (double)busy / alive * 100 >= 70) && alive + pthis->m_thread_min <= pthis->m_thread_max) {
			for(flag=0,add=0;flag<pthis->m_thread_max && add<= pthis->m_thread_min;flag++) {
				if(pthis->m_ctids[flag] == 0 || pthis->isThreadAlive(pthis->m_ctids[flag])) {
					pthread_create(&pthis->m_ctids[flag], NULL, Customer_thread,(void*)pthis);
					add++;
					pthread_mutex_lock(&pthis->m_lock);
					++(pthis->m_thread_alive);
					pthread_mutex_unlock(&pthis->m_lock);
				}
			}
		}
		if(busy*2 <= alive - busy && alive - pthis->m_thread_min >= pthis->m_thread_min) {
			pthread_mutex_lock(&pthis->m_lock);
			pthis->m_kill_number = pthis->m_thread_min;
			pthread_mutex_unlock(&pthis->m_lock);
			for(int i=0;i<pthis->m_thread_min;i++) {
				pthread_cond_signal(&pthis->m_Not_Empty);
			}
		}
		sleep(TIMEOUT);
	}
	printf("thread shutdown its 0 , manager 0x%x exit..\n",(unsigned int)pthread_self());
	return nullptr;
}
int CThreadPool::thread_pool_destroy(){
	//释放资源
	pthread_mutex_destroy(&m_lock);
	pthread_cond_destroy(&m_Not_Empty);
	pthread_cond_destroy(&m_Not_Full);

	delete[] m_ctids;
	delete[] m_queue;
	m_ctids = nullptr;
	m_queue = nullptr;
	return 0;
}

bool CThreadPool::isThreadAlive(pthread_t tid) {
	int err =pthread_kill(tid, 0);
	if(err == ESRCH) {
		return true;
	}
	return false;
}
