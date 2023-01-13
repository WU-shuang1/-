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

//����3������������2��������
pthread_mutex_t lock_a, lock_b;
pthread_cond_t cd_a, cd_b, cd_c;

//��������������
char buf1[SIZE];
char buf2[SIZE];

//��֤buf1����Ϊ��λ�����һ���ַ�Ϊ\n
int myread(char* buf,int len) {
	int sum = 0;
	int i;
	//���ֻ��һ�����ݾ�û��\n��ֱ�ӷ��أ����һ�����ݳ����˻�������С�޷�ɾ����ֻ�ܰѻ��������
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

//��ȡ�����������һ�У���ȡ�꽫���һ��ɾ��
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
	//myreadɾ�����һ�У���ĩβ��\n��Ϊ��ģ����ֻʣ���һ�У������ֶ�ɾ��
	buf[len - 1] = 'a';
	if (myread(buf, len) == 0) {
		for (i = len - 1; i >= 0; i--) {
			buf[i] = '\0';
		}
	}
	return 1;
}

//A�̺߳�C�߳̿���ͬʱ��������ʱB���ܷ�������������B����ʱ���������̹߳���
void* thread_a(void* arg) {
	int fd = open("ERROR.log", O_RDONLY);
	int len = lseek(fd, 0, SEEK_END);	//�ļ��ܳ���
	lseek(fd, 0, SEEK_SET);
	int sum = 0;	//��ȡ���ݵ��ܳ���
	int temp;
	while (len > sum) {
		pthread_mutex_lock(&lock_a);
		while (strlen(buf1) != 0) {
			pthread_cond_wait(&cd_a, &lock_a);
		}
		sum += read(fd, buf1, SIZE);
		//��ȡ�����ݽ��������ڲ�������һ��ɾ��Ȼ���ļ���дλ���ƶ����п�ͷ
		temp = myread(buf1, strlen(buf1));
		sum -= temp;
		lseek(fd, -temp, SEEK_CUR);
		//��ȡ��һ�κ���B�߳�
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
		//д�뻺����2֮����A��C�߳�
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
		//��ջ�����
		memset(buf2, 0, SIZE);
		//д����һ�κ���B�߳�
		printf("thread C 0x%x success once.\n", (unsigned int)pthread_self());
		pthread_cond_signal(&cd_b);
		pthread_mutex_unlock(&lock_b);
	}
	close(fd);
}

int main() {
	pthread_t tid_a, tid_b, tid_c;
	//��ʼ��������������
	pthread_mutex_init(&lock_a, NULL);
	pthread_mutex_init(&lock_b, NULL);
	pthread_cond_init(&cd_a, NULL);
	pthread_cond_init(&cd_b, NULL);
	pthread_cond_init(&cd_c, NULL);

	//�����߳�
	pthread_create(&tid_a, NULL, &thread_a, NULL);
	pthread_create(&tid_b, NULL, &thread_b, NULL);
	pthread_create(&tid_c, NULL, &thread_c, NULL);

	//���������߳�
	pthread_join(tid_a, NULL);
	//��a�̻߳���֮�󣬻���b�߳���Ҫ��buf1������֮��ſ���
	usleep(100000);
	pthread_cancel(tid_b);
	pthread_join(tid_b, NULL);
	//��b�̻߳���֮�󣬻���c�߳���Ҫ��buf2������֮��ſ���
	usleep(100000);
	pthread_cancel(tid_c);
	pthread_join(tid_c, NULL);

	//����������������
	pthread_mutex_destroy(&lock_a);
	pthread_mutex_destroy(&lock_b);
	pthread_cond_destroy(&cd_a);
	pthread_cond_destroy(&cd_b);
	pthread_cond_destroy(&cd_c);

	printf("file Succseeful\n");

	return 0;
}
