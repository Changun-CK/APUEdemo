#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define THREAD_NUM 10

// 一条汇编指令能够实现的,叫原子操作,注意变量的自增自减并不是原子操作
// 原子操作的应用非常广泛,原子操作只适合单条指令,跟spinlock等锁不一样
// CAS --> compare and swap, 如何实现线程安全单例模式?

pthread_mutex_t mutex; // 互斥锁,会切换,会做切换,适用于操作时间长的事务
pthread_spinlock_t spinlock; // 自旋锁,忙等待,不会切换,只等待释放锁,适用于操作简单时间短的事务,何为长何为短呢,以线程切换的时间来衡量

int inc(int *value, int add);

void *thread_proc(void *arg);

int main(int argc, char *argv[])
{
	// 准备10个线程
	pthread_t thread_id[THREAD_NUM] = { 0 };

	int count = 0;

	pthread_mutex_init(&mutex, NULL); // 初始化互斥锁
	pthread_spin_init(&spinlock, PTHREAD_PROCESS_SHARED); // 初始化自旋锁

	int i = 0;
	for (i = 0; i < THREAD_NUM; i++)
	{
		pthread_create(&thread_id[i], NULL, thread_proc, &count);
	}

	for (i = 0; i < 100; i++)
	{
		printf("count --> %d\n", count);
		sleep(1);
	}

	return 0;
}

void *thread_proc(void *arg)
{
	int *pcount = (int *)arg;

	int i = 0;
	while (i++ < 3000000)
	{
#if 0
		(*pcount)++; // 不使用锁
#elseif 0
		pthread_mutex_lock(&mutex); // 使用互斥锁
		(*pcount)++;
		pthread_mutex_unlock(&mutex);
#elseif 0
		pthread_spin_lock(&spinlock); // 使用自旋锁
		(*pcount)++;
		pthread_spin_unlock(&spinlock);
#else
		inc(pcount, 1); // 原子操作
#endif
		usleep(1);
	}
}

int inc(int *value, int add)
{
	int old;

	// 汇编指令
	__asm__ volatile(
		"lock; xaddl %2, %1;"
		: "=a" (old)
		: "m" (*value), "a"(add)
		: "cc", "memory"
	);

	return old;
}
