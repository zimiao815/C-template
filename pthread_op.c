#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <syslog.h>
#include <errno.h>
#include <stdlib.h>
#include <malloc.h>
#include <stdlib.h>


#include "config.h"
#include "debug.h"
#include "safe.h"
#include "file_op.h"
#include "sock.h"
#include "tick.h"
#include "pthread_op.h"
#include "http.h"
#include "curl_op.h"
#include "linkedlist.h"



#define BUFFER_SIZE 32 // 缓冲区数量
#define OVER ( - 1)



struct prodcons
{
	t_filemac findmac[BUFFER_SIZE];
	pthread_mutex_t lock; /* 互斥体lock 用于对缓冲区的互斥操作 */
	int readpos, writepos; /* 读写指针*/
	pthread_cond_t notempty; /* 缓冲区非空的条件变量 */
	pthread_cond_t notfull; /* 缓冲区未满的条件变量 */
};

struct prodcons Getmacbuf;



/* 初始化缓冲区结构 */
void init(struct prodcons *b)
{
	pthread_mutex_init(&b->lock, NULL);
	pthread_cond_init(&b->notempty, NULL);
	pthread_cond_init(&b->notfull, NULL);
	b->readpos = 0;
	b->writepos = 0;
	memset(b->findmac,0,sizeof(b->findmac));
}
/* 将产品放入缓冲区,这里是存入一个整数*/

void put(struct prodcons *b, t_filemac * data)
{
	pthread_mutex_lock(&b->lock);
	/* 等待缓冲区未满*/
	if ((b->writepos + 1) % BUFFER_SIZE == b->readpos)
	{
        debug(LOG_DEBUG,"put:pthread_cond_wait notfull");
		pthread_cond_wait(&b->notfull, &b->lock);
	}
	/* 写数据,并移动指针 */
	memcpy(&b->findmac[b->writepos] , data,sizeof( t_filemac));
	b->writepos++;
	if (b->writepos >= BUFFER_SIZE) b->writepos = 0;

	/* 设置缓冲区非空的条件变量*/
	pthread_cond_signal(&b->notempty);
	pthread_mutex_unlock(&b->lock);
}


/* 从缓冲区中取出整数*/
void get(struct prodcons *b,t_filemac * data)
{

	pthread_mutex_lock(&b->lock);
	/* 等待缓冲区非空*/
	if (b->writepos == b->readpos)
	{
       debug(LOG_DEBUG,"get:pthread_cond_wait notempty");
	   pthread_cond_wait(&b->notempty, &b->lock);
	}
	/* 读数据,移动读指针*/
	memcpy(data , &b->findmac[b->readpos],sizeof( t_filemac));
	b->readpos++;
	if (b->readpos >= BUFFER_SIZE)
	    b->readpos = 0;
	/* 设置缓冲区未满的条件变量*/
	pthread_cond_signal(&b->notfull);
	pthread_mutex_unlock(&b->lock);
	return ;
}

/** @internal

*/
static int
check_file_src(char *line)
{
    if (strstr(line, "wifideal") !=NULL ) {
        return 1;
    }
    else if (strstr(line, "wifidog") !=NULL ) {
        return 2;
    }

    else if (strstr(line, "dpi") !=NULL ) {
        return 3;
    }

    return -1;
}




void *producer(void *data)
{
   FILE *fp;
   s_config *config = config_get_config();
   int updatetime=config->updatetime*60;
   int now,lasttime;
   t_filemac *pp;
   t_dir_filenames *llll;
   char content[1024];
   t_Monitor_mac *lb;
   t_Monitor_server_mac *sb;
   int filetype;
   char array[8];


   t_dir_filename * p_s_dir_filename=NULL;;
   p_s_dir_filename=get_dir_filename();
   dir_filename_init(p_s_dir_filename);
   while(1){
	   int now = time(NULL);
	   if( now - lasttime >= updatetime ){
		   saved_server_monitor_mac(); //�ӷ������mac�б�������
		   lasttime=now;
	   }

	   /********************����һ����һ�������ļ�����************************/

	   for(llll=p_s_dir_filename->pfnames;llll!=NULL;llll=llll->next){
			debug(LOG_DEBUG, "substring=%s\n",llll->pfilename);
			free(llll->pfilename);
			llll->pfilename=NULL;
			free(llll);
		}
        /********************���»�ȡ�ļ���***********************************/
        p_s_dir_filename=get_dir_filename();
        dir_filename_init(p_s_dir_filename);
	    trave_dir(config->srcpath,p_s_dir_filename);
		for(llll=p_s_dir_filename->pfnames;llll!=NULL;llll=llll->next){
						  /* �����ڶ�ȡ���ļ� */
			  fp = fopen(llll->pfilename , "r");
			  if(fp == NULL) {
				 debug(LOG_DEBUG, "%s opened fails\n",llll->pfilename);
				 return NULL;
			  }
			  debug(LOG_DEBUG, "substring=%s\n",llll->pfilename);
			  filetype=check_file_src(llll->pfilename);
			  memset(array,0,sizeof(array));
			  if (filetype==1){
					strcpy(array,"2|1");
			  }else if(filetype==2) {
                    strcpy(array,"1|5");
			  }else if(filetype==3) {
                    strcpy(array,"3|1");
			  }else{
				fclose(fp);
				continue;
			  }
			 while( fgets (content, sizeof(content), fp)!=NULL ) {
				/* ���׼��� stdout д������ */
				debug(LOG_DEBUG,"content=%s\n",content);

				pp = parse_extract_found_mac(content,array);
				memset(content,0,sizeof(content));
				debug(LOG_DEBUG,"pp->pmac=%s\n",pp->pmac);
				debug(LOG_DEBUG,"pp->timest=%s\n",pp->timest);
				sb=server_binseach(config->P_t_Monitor_server_mac,pp->pmac);
				if(sb!=NULL){//�ҵ��˲���mac�����б��
					pp->flag=1;
					put(&Getmacbuf,pp);
				}
				lb=local_binseach(config->P_t_Monitor_mac,pp->pmac);
				if(lb!=NULL){
					pp->flag=2;
					put(&Getmacbuf,pp);
				}
				free(pp);
				pp=NULL;

			 }

			 fclose(fp);
		 }
        debug(LOG_DEBUG,"sleep(20);\n");
	sleep(20);
   	}
return NULL;
}

void *consumer(void *data)
{
	t_filemac  d;
    char arr_url[1024];
    struct CurlMemoryRx *pcurl_mem;

	s_config *config = config_get_config();

	while (1)
	{
        filemac_init(&d);
		get(&Getmacbuf,&d);
		memset(arr_url,0,sizeof(arr_url));
		if(d.flag==1 || d.flag==2){
			sprintf(arr_url,"%s%s%s%s%s%s%s%s%s%s",
				config->serverurl,
				"?mac=",d.pmac,
				"&time=",d.timest,
				"&deviceId=",config->deviceid,
				"&devicephone=",config->P_t_Phonenum->plocal_phone,
				"&type");

		}else {
           continue;
		}
		debug(LOG_DEBUG,"curl requite arr_url=%s\n",arr_url);
		pcurl_mem=curl_getUrl(arr_url);
		debug(LOG_DEBUG,"curl return string=%s\n",pcurl_mem->memory);
		free(pcurl_mem->memory);//这块内存是在curl_getUrl的回调函数中产生的

        debug(LOG_DEBUG,"sleep(30);\n");
      sleep(30);

	}
  return NULL;
}

int pthread_op(void)
{
	pthread_t th_a, th_b;
	void *retval;
	init(&Getmacbuf);
	/* 创建生产者和消费者线程*/
	pthread_create(&th_a, NULL, producer, 0);
	pthread_create(&th_b, NULL, consumer, 0);
	/* 等待两个线程结束*/
	pthread_join(th_a, &retval);
	pthread_join(th_b, &retval);
	return 0;
}



