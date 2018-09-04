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
#include <pthread.h>
#include <curl/curl.h>


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


#define FILE_LOG "./files.log"



#ifndef FALSE
#define FALSE   (0)
#define false   (0)
#endif

#ifndef TRUE
#define TRUE    (!FALSE)
#define true    (!false)
#endif





int main(int argc, char* argv[])
{
	pthread_t th_tick;
	void *retval=NULL;
	FILE *fp;
	char content[1024];
    t_dir_filenames *llll;
	t_filemac *pp;

	s_config *config = config_get_config();
	config_init();
	config_read(config->configfile);
	debug(LOG_DEBUG, "++++++++++config_read end\n");
	pthread_op();
    while(1){
      sleep(600);
    }

	return 0;

}
