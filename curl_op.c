#include <stdio.h>
#include <errno.h>
#include <syslog.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <syslog.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <curl/curl.h>

#include "config.h"
#include "debug.h"
#include "curl_op.h"
#include "safe.h"
#include "linkedlist.h"
#include "unit.h"




struct CurlMemoryRx chunk;

/**
* @brief libcurl接收到数据时的回调函数
*
* 将接收到的数据保存到本地文件中，同时显示在控制台上。
*
* @param [in] contents 接收到的数据所在缓冲区
* @param [in] size 数据长度
* @param [in] nmemb 数据片数量
* @param [in/out] userp 用户自定义指针
* @return 获取的数据长度
*/
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct CurlMemoryRx *mem = (struct CurlMemoryRx *)userp;

  mem->memory =(char *) safe_realloc(mem->memory, mem->size + realsize + 1);
  if(mem->memory == NULL) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }

  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;

  return realsize;
}

/**
* @brief 初始化crul，并且获取url返回值
*
* 将接收到的数据保存到 chunk结构体中，
*
* @param [in] urladdr 请求的地址

* @return 获取的数据内存地址和数据长度（字节个数）
*/




struct CurlMemoryRx * curl_getUrl( char * urladdr )
{
    CURL *curl;
    CURLcode res;
    char * purladdr=urladdr;


    chunk.memory = (char *)safe_malloc( sizeof(char) );  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    curl = curl_easy_init();    // 初始化
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL,purladdr);
		  /* complete connection within 10 seconds */
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);//连接阶段超时
		  /* complete within 20 seconds */
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);//请求超时
		//curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);//多线程需要设置为1 默认为了 屏蔽超时产生的alarm信号
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		//curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp); //将返回的html主体数据输出到fp指向的文件
		//curl_easy_setopt(curl, CURLOPT_HEADER, 0L);  //若启用，会将头文件的信息作为数据流输出
        //curl_easy_setopt(curl, CURLOPT_HEADERDATA, fp); //将返回的http头输出到fp指向的文件
		//curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); //设置CURLOPT_FOLLOWLOCATION为true，则会跟踪爬取重定向页面，否则，不会跟踪重定向页面。
        res = curl_easy_perform(curl);   // 执行
		  /* check for errors */
		if(res != CURLE_OK) {
			//curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
		    debug(LOG_ERR, "Error:url_easy_perform fail %s !\n",curl_easy_strerror(res));
		}
		else {
		/*
		 * Now, our chunk.memory points to a memory block that is chunk.size
		 * bytes big and contains the remote file.
		 *
		 * Do something nice with it!
		 */
            debug(LOG_DEBUG, "%lu bytes retrieved : %s\n", (unsigned long)chunk.size,chunk.memory);
		}
		  /* cleanup curl stuff */
		curl_easy_cleanup(curl);


        return &chunk;
    }
}


/**
* @brief 解析服务器返回的mac布控信息，然后保存到config.c的配置实体中
*
*
*
* @param [in] ptr 服务器返回的mac数据存储在ptr->memory指向的空间
* @param [out] pconfig 将服务器的数据保存于此指针指向的结构体，实体空间在此函数中分配
* @return 无
*/


static void
parse_server_monitor_mac(struct CurlMemoryRx * ptr ,s_config * pconfig)
{

	char * ptrcopy = NULL;
    char *p_str = NULL;
    char *tmp=NULL;
	t_Monitor_server_mac *p = NULL;
    debug(LOG_DEBUG, "Parsing string [%s] for deamon", ptr);

    /* strsep modifies original, so let's make a copy */
    ptrcopy = safe_strdup(ptr->memory);

    while ((p_str = strsep(&ptrcopy, "|"))) {
	        /* Skip leading spaces. */
        while (*p_str != '\0' && (isblank(*p_str) || *p_str == '\"') ) {
            p_str++;
        }
        if (*p_str == '\0') {  /* Equivalent to strcmp(p_str, "") == 0 */
            continue;
        }
        /* Remove any trailing blanks. */
        tmp = p_str;
        while (*tmp != '\0' && !isblank(*tmp) &&  *tmp != '\"') {
            tmp++;
        }
        if (*tmp != '\0' && (isblank(*tmp) || *tmp == '\"') ) {
            *tmp = '\0';
        }

        debug(LOG_DEBUG, "Adding string list [%s] to t_Monitor_server_mac", p_str);

		if (is_valid_mac_addr(p_str)!=0){
			continue;
		}

	    p = (t_Monitor_server_mac *)safe_malloc(sizeof(t_Monitor_server_mac));
	    p->pserver_mac = safe_strdup(p_str);//实际存储数据的空间在此处被申请


		if (pconfig->P_t_Monitor_server_mac == NULL) {
			pconfig->P_t_Monitor_server_mac = safe_malloc(sizeof(t_Monitor_server_mac));
			p->next = NULL;
			pconfig->P_t_Monitor_server_mac = p;
		} else {
			p->next = pconfig->P_t_Monitor_server_mac;
			pconfig->P_t_Monitor_server_mac = p;
		}





	}

	free(ptrcopy);
}











void saved_server_monitor_mac( void )
{

    char arr_url[1024];
    struct CurlMemoryRx *pcurl_mem;
	t_Monitor_server_mac  *print;
	s_config *config = config_get_config();
    memset(arr_url,0,sizeof(arr_url));
	sprintf(arr_url,"%s%s%s",config->serverurl,GET_SERVER_MONITOR_MACS,config->deviceid);
    pcurl_mem=curl_getUrl(arr_url);
    parse_server_monitor_mac(pcurl_mem,config);
    free(pcurl_mem->memory);//这块内存是在curl_getUrl的回调函数中产生的
    config->P_t_Monitor_server_mac=sort_server_mac_list(config->P_t_Monitor_server_mac);

	for (print=config->P_t_Monitor_server_mac;print!=NULL;print=print->next){
		debug(LOG_DEBUG, "server_mac=%s\n",print->pserver_mac);
	}
}

/*
bool postUrl(char *filename)
{
    CURL *curl;
    CURLcode res;
    FILE *fp;
    if ((fp = fopen(filename, "w")) == NULL)
        return false;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "/tmp/cookie.txt"); // 指定cookie文件
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "&logintype=uid&u=xieyan&psw=xxx86");    // 指定post内容
        //curl_easy_setopt(curl, CURLOPT_PROXY, "10.99.60.201:8080");
        curl_easy_setopt(curl, CURLOPT_URL, " http://mail.sina.com.cn/cgi-bin/login.cgi ");   // 指定url
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
    fclose(fp);
    return true;
}
*/

void free_monitor_servermac_list( void )
{
   s_config *config = config_get_config();
   t_Monitor_server_mac  *p_t;
   for(p_t=config->P_t_Monitor_server_mac; p_t!=NULL; p_t=p_t->next){
       free(p_t->pserver_mac);
	   p_t->pserver_mac=NULL;
	   free(p_t);
   }
}



