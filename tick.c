#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include "tick.h"

static int sec_count; 
int Sec = 1;


typedef void (*MyTimerHandle)(void* arg);//函数指针

typedef struct My_Timer_Args
{
	int sec;
	MyTimerHandle MTHandle;
}MyTimerArgs;

MyTimerArgs* G_MT[50];//结构体指针数组，存放实体的地址
static int TimerCount=0; 


void My_Set_Timer(MyTimerArgs* arg)
{
	G_MT[TimerCount] = arg;
	TimerCount++;
}

void DealWithSigalrm(int sig)
{   
	int i;
	sec_count++;
	for(i=0;i<TimerCount;i++)
	{
		if((sec_count%(G_MT[i]->sec)) == 0)
			G_MT[i]->MTHandle(NULL);
	}
}


static void FiveSecondHandle(void* arg)
{
	printf("Five Second  count:%d\n",sec_count);
}


static void TenSecondHandle(void* arg)
{
	printf("Ten Second count:%d\n",sec_count);
}


int SysTick() //系统定时器
{
	struct itimerval value, ovalue;

	if(signal(SIGALRM,DealWithSigalrm) == SIG_ERR){
		printf("error regist sigalrm\n");
		return -1;
	}
	
	MyTimerArgs mta1,mta2;
	mta1.sec = 5;
	mta1.MTHandle = FiveSecondHandle;
	mta2.sec =10;
	mta2.MTHandle = TenSecondHandle;
	My_Set_Timer(&mta1);
	My_Set_Timer(&mta2);


	value.it_value.tv_sec = Sec; //首先延时Sec产生SIGALRM
	value.it_value.tv_usec = 0;
	value.it_interval.tv_sec = Sec; //然后每隔Sec产生SIGALRM
	value.it_interval.tv_usec = 0;
	/*先对it_value倒计时，当it_value为零时触发信号，然后重置为it_interval，继续对it_value倒计时，一直这样循环下去。*/
	setitimer(ITIMER_REAL, &value, &ovalue); //ITIMER_REAL ：以系统真实的时间来计算，它送出SIGALRM信号
	while(1)
	{
		sleep(Sec);
	}
  return 0;
}

void *pthread_systick(void *data)
{
	struct itimerval value, ovalue;

	if(signal(SIGALRM,DealWithSigalrm) == SIG_ERR){
		printf("error regist sigalrm\n");
		return NULL;
	}
	
	MyTimerArgs mta1,mta2;
	mta1.sec = 5;
	mta1.MTHandle = FiveSecondHandle;
	mta2.sec =10;
	mta2.MTHandle = TenSecondHandle;
	My_Set_Timer(&mta1);
	My_Set_Timer(&mta2);


	value.it_value.tv_sec = Sec; //首先延时Sec产生SIGALRM
	value.it_value.tv_usec = 0;
	value.it_interval.tv_sec = Sec; //然后每隔Sec产生SIGALRM
	value.it_interval.tv_usec = 0;
	/*先对it_value倒计时，当it_value为零时触发信号，然后重置为it_interval，继续对it_value倒计时，一直这样循环下去。*/
	setitimer(ITIMER_REAL, &value, &ovalue); //ITIMER_REAL ：以系统真实的时间来计算，它送出SIGALRM信号
	while(1)
	{
		sleep(Sec);
	}
  return NULL;
}