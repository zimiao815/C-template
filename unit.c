#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include<net/if.h>
#include<net/if_arp.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <regex.h>



#include "unit.h"
#include "debug.h"

#define ETH_ALEN 6

#define MEM_512KB 512*1024
#define MEM_1024KB 1024*1024









int is_valid_mac_addr(char* mac)
{

	int status;
	const char * pattern = "^([A-Fa-f0-9]{2}[-,:]){5}[A-Fa-f0-9]{2}$";

    //REG_EXTENDED  用POSIX正则表达式扩展规范来翻译正则表达式regex。如果没有设置REG_EXTENDED标志，则只支持POSIX正则表达式基本规范
	//REG_NEWLINE 匹配满足正则表达式的任何字符符但不匹配换行符

	const int cflags = REG_EXTENDED | REG_NEWLINE;

	char ebuf[128];
	regmatch_t pmatch[1];
	int nmatch = 10;
	regex_t reg;


	status = regcomp(&reg, pattern, cflags);//编译正则模式
	if(status != 0) {
		regerror(status, &reg, ebuf, sizeof(ebuf));
		debug(LOG_DEBUG, "regcomp fail: %s , pattern '%s' \n",ebuf, pattern);
		//fprintf(stderr, "regcomp fail: %s , pattern '%s' \n",ebuf, pattern);
		goto failed;
	}

	status = regexec(&reg, mac, nmatch, pmatch,0);//执行正则表达式和缓存的比较,
	if(status != 0) {
		regerror(status, &reg, ebuf, sizeof(ebuf));
		debug(LOG_DEBUG, "regexec fail: %s , mac:\"%s\" \n", ebuf, mac);
		//fprintf(stderr, "regexec fail: %s , mac:\"%s\" \n", ebuf, mac);
		goto failed;
	}


	regfree(&reg);
	return 0;

failed:
	regfree(&reg);
	return -1;
}









int check_system_endian()
{
   union {
     int m;
     char n;
  }end;
  end.m=1;
  /*big endian 0; little endian 1*/
  return (end.n==1);

}

int strToHex(char *ch, char *hex)
{
  unsigned char value,high=0,low=0;
  unsigned char tmp = 0;
  if(ch == NULL || hex == NULL){
    return -1;
  }
  if(strlen(ch) == 0){
    return -2;
  }
  while(*ch){
	if( (*ch) >='0' && (*ch)<='9' ){
		*ch=*ch-'0';
	}
	else if( (*ch) >='A' && (*ch)<='F' ){
		*ch=*ch-'A'+10;
	}
	else if( (*ch) >='a' && (*ch)<='f' ){
		*ch=*ch-'a'+10;
	}
	else{
		ch++;
		continue;
	}
    tmp = *ch;
    high = tmp << 4;
	ch++;
	if( (*ch) >='0' && (*ch)<='9' ){
		*ch=*ch-'0';
	}
	else if( (*ch) >='A' && (*ch)<='F' ){
		*ch=*ch-'A'+10;
	}
	else if( (*ch) >='a' && (*ch)<='f' ){
		*ch=*ch-'a'+10;
	}
	else{
		ch++;
		continue;
	}
	tmp = *ch;
    low = tmp & 0x0f;
	value= high | low;
	*hex=value;
	high=0;low=0;
	tmp=0;
	ch++;
  }
  return 0;
}




int get_macAddr( unsigned char * macaddr )
{
	char *device="eth0"; //eth0是网卡设备名
	int i,s;
	s = socket(AF_INET,SOCK_DGRAM,0); //建立套接口
	struct ifreq req;
	int err;
	strcpy(req.ifr_name,device); //将设备名作为输入参数传入
	err=ioctl(s,SIOCGIFHWADDR,&req); //执行取MAC地址操作
	close(s);
	if( err != -1 )  {
		memcpy(macaddr,req.ifr_hwaddr.sa_data,ETH_ALEN); //取输出的MAC地址
	}
	return err;
}



int limit_memory(int argc, char **argv )
{


     //#define MEM_512KB 512*1024
     //#define MEM_1024KB 1024*1024

	static int inum = 0;
	struct rlimit lim = {0};
	getrlimit(RLIMIT_STACK, &lim);
	if (lim.rlim_cur == MEM_512KB && lim.rlim_max == MEM_512KB) {
		if (inum++ != 0)
			return 0;
	} else {
		lim.rlim_cur = MEM_512KB;
		lim.rlim_max = MEM_512KB;
		if (setrlimit(RLIMIT_STACK, &lim) == -1) {
			//debug(LOG_ERR, "the function[%s] line[%d] rlimit failed\r\n", __FUNCTION__, __LINE__);
		}
		pid_t pid;
		pid = vfork();   //vfork:子进程和父进程共享资源，子进程调用exit或者exec族函数之后才执行父进程
		                 //如果子进程依赖于父进程的条件则不要使用vfork，会出现死锁，要使用fork
		if(pid<0)
			printf("error in fork!\n");
		else if(pid == 0)
		{
			printf("I am the child process,ID +++ is %d\n",getpid());
			execve(argv[0], argv, NULL);

		}
		else
		{
			printf("I am the parent process,ID is %d is exit \n",getpid());
			_exit(EXIT_SUCCESS);
		}


	}

    return 0;
}








