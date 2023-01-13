#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include <signal.h>
#define SIZE 1024

//定义3个条件变量和2个互斥锁
pthread_mutex_t lock_a, lock_b;
pthread_cond_t cd_a, cd_b, cd_c;

//声明两个缓冲区
char buf1[SIZE];
char buf2[SIZE];

//保证buf1以行为单位，最后一个字符为\n
int myread(char* buf,int len) {
	int sum = 0;
	int i;
	//如果只有一行数据就没有\n，直接返回，如果一行数据超过了缓冲区大小无法删除，只能把缓冲区变大
	if (strrchr(buf, '\n') == NULL) {
		return 0;
	}
	for (i = len - 1; i >= 0; i--) {
		if (buf[i] == '\n') {
			return sum;
		}
		else {
			sum++;
			buf[i] = '\0';
		}
	}
	return 0;
}

//读取缓冲区内最后一行，读取完将最后一行删除
int fun(char* temp, char* buf, int len) {
	int i;
	if (len == 0)
		return 0;
	for (i = len - 2; i >= 0; i--) {
		if (buf[i] == '\n') {
			i++;
			break;
		}
	}
	if (i == -1)
		i = 0;
	for (int j = 0; i < len; j++,i++) {
		temp[j] = buf[i];
	}
	//myread删除最后一行，将末尾的\n改为别的，如果只剩最后一行，自行手动删除
	buf[len - 1] = 'a';
	if (myread(buf, len) == 0) {
		for (i = len - 1; i >= 0; i--) {
			buf[i] = '\0';
		}
	}
	return 1;
}

//A线程和C线程可以同时工作，此时B不能访问这两把锁，B工作时其他两个线程挂起
void* thread_a(void* arg) {
	int fd = open("ERROR.log", O_RDONLY);
	int len = lseek(fd, 0, SEEK_END);	//文件总长度
	lseek(fd, 0, SEEK_SET);
	int sum = 0;	//读取数据的总长度
	int temp;
	while (len > sum) {
		pthread_mutex_lock(&lock_a);
		while (strlen(buf1) != 0) {
			pthread_cond_wait(&cd_a, &lock_a);
		}
		sum += read(fd, buf1, SIZE);
		//读取完数据将缓冲区内不完整的一行删除然后将文件读写位置移动回行开头
		temp = myread(buf1, strlen(buf1));
		sum -= temp;
		lseek(fd, -temp, SEEK_CUR);
		//读取完一次后唤醒B线程
		printf("thread A 0x%x success once.\n", (unsigned int)pthread_self());
		pthread_cond_signal(&cd_b);
		pthread_mutex_unlock(&lock_a);
	}
	close(fd);
}

void* thread_b(void* arg) {
	char temp[SIZE];
	memset(temp, 0, SIZE);
	while (1) {
		pthread_mutex_lock(&lock_a);
		while (strlen(buf1) == 0) {
			pthread_cond_wait(&cd_b, &lock_a);
		}
		pthread_mutex_lock(&lock_b);
		while (strlen(buf2) != 0) {
			pthread_cond_wait(&cd_b, &lock_b);
		}
		//写入缓冲区2之后唤醒A、C线程
		while (fun(temp,buf1,strlen(buf1))) {
			if ((NULL != strstr(temp, "E CamX") || NULL != strstr(temp, "E CHIUSECASE"))) {
				strcat(buf2, temp);
			}
			memset(temp, 0, SIZE);
		}
		printf("thread B 0x%x success once.\n", (unsigned int)pthread_self());
		pthread_cond_signal(&cd_c);
		pthread_mutex_unlock(&lock_b);
		pthread_cond_signal(&cd_a);
		pthread_mutex_unlock(&lock_a);
	} 
}
void* thread_c(void* arg) {
	int fd = open("./result", O_WRONLY | O_CREAT | O_APPEND,0664);
	while (1) {
		pthread_mutex_lock(&lock_b);
		while (strlen(buf2) == 0) {
			pthread_cond_wait(&cd_c, &lock_b);
		}
		write(fd, buf2, strlen(buf2));
		//清空缓冲区
		memset(buf2, 0, SIZE);
		//写入完一次后唤醒B线程
		printf("thread C 0x%x success once.\n", (unsigned int)pthread_self());
		pthread_cond_signal(&cd_b);
		pthread_mutex_unlock(&lock_b);
	}
	close(fd);
}

int main() {
	pthread_t tid_a, tid_b, tid_c;
	//初始化条件变量和锁
	pthread_mutex_init(&lock_a, NULL);
	pthread_mutex_init(&lock_b, NULL);
	pthread_cond_init(&cd_a, NULL);
	pthread_cond_init(&cd_b, NULL);
	pthread_cond_init(&cd_c, NULL);

	//创建线程
	pthread_create(&tid_a, NULL, &thread_a, NULL);
	pthread_create(&tid_b, NULL, &thread_b, NULL);
	pthread_create(&tid_c, NULL, &thread_c, NULL);

	//阻塞回收线程
	pthread_join(tid_a, NULL);
	//当a线程回收之后，回收b线程需要等buf1用完了之后才可以
	usleep(100000);
	pthread_cancel(tid_b);
	pthread_join(tid_b, NULL);
	//当b线程回收之后，回收c线程需要等buf2用完了之后才可以
	usleep(100000);
	pthread_cancel(tid_c);
	pthread_join(tid_c, NULL);

	//销毁锁和条件变量
	pthread_mutex_destroy(&lock_a);
	pthread_mutex_destroy(&lock_b);
	pthread_cond_destroy(&cd_a);
	pthread_cond_destroy(&cd_b);
	pthread_cond_destroy(&cd_c);

	printf("file Succseeful\n");

	return 0;
}
