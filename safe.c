#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>
#include <time.h>

#include "safe.h"
#include "debug.h"






void *
safe_malloc(size_t size)
{
    void *retval = NULL;
    retval = malloc(size);
    if (!retval) {
        debug(LOG_CRIT, "Failed to malloc %d bytes of memory: %s.  Bailing out", size, strerror(errno));
        exit(1);
    }
    memset(retval, 0, size);
    return (retval);
}


/** Re-allocates memory to a new larger allocation.
 * DOES NOT ZERO the added RAM
 * Original pointer is INVALID after call
 * Dies on allocation failures.
 * @param ptr A pointer to a current allocation from safe_malloc()
 * @param newsize What size it should now be in bytes
 * @return pointer to newly allocation ram
 */
void *
safe_realloc(void *ptr, size_t newsize)
{
    void *retval = NULL;
    retval = realloc(ptr, newsize);
    if (NULL == retval) {
        debug(LOG_CRIT, "Failed to realloc buffer to %d bytes of memory: %s. Bailing out", newsize, strerror(errno));
        exit(1);
    }
    return retval;
}



/** Duplicates a string or die if memory cannot be allocated
 * @param s String to duplicate
 * @return A string in a newly allocated chunk of heap.
 */
char *
safe_strdup(const char *s)
{
    char *retval = NULL;
    if (!s) {
        debug(LOG_CRIT, "safe_strdup called with NULL which would have crashed strdup. Bailing out");
        exit(1);
    }
    retval = strdup(s);
    if (!retval) {
        debug(LOG_CRIT, "Failed to duplicate a string: %s.  Bailing out", strerror(errno));
        exit(1);
    }
    return (retval);
}
char * strdup_len (const char *s,int slen)
{
	//size_t len =strlen (s) + 1;
	int len = slen + 1;
	void *new =malloc (len);
	if (new == NULL)
		return NULL;
	return (char *)memcpy (new, s, len);
}

char *
safe_strdup_len(const char *s,int len)
{
    char *retval = NULL;
    if (!s) {
        debug(LOG_CRIT, "safe_strdup called with NULL which would have crashed strdup. Bailing out");
        exit(1);
    }
    retval = strdup_len(s,len);
    if (!retval) {
        debug(LOG_CRIT, "Failed to duplicate a string: %s.  Bailing out", strerror(errno));
        exit(1);
    }
    return (retval);
}

/** Sprintf into a newly allocated buffer
 * Memory MUST be freed. Dies if memory cannot be allocated.
 * @param strp Pointer to a pointer that will be set to the newly allocated string
 * @param fmt Format string like sprintf
 * @param ... Variable number of arguments for format string
 * @return int Size of allocated string.
 */
int
safe_asprintf(char **strp, const char *fmt, ...)
{
    va_list ap;
    int retval;

    va_start(ap, fmt);
    retval = safe_vasprintf(strp, fmt, ap);
    va_end(ap);

    return (retval);
}

/** Sprintf into a newly allocated buffer
 * Memory MUST be freed. Dies if memory cannot be allocted.
 * @param strp Pointer to a pointer that will be set to the newly allocated string
 * @param fmt Format string like sprintf
 * @param ap pre-digested va_list of arguments.
 * @return int Size of allocated string.
 */
int
safe_vasprintf(char **strp, const char *fmt, va_list ap)
{
    int retval;

    retval = vasprintf(strp, fmt, ap);

    if (retval == -1) {
        debug(LOG_CRIT, "Failed to vasprintf: %s.  Bailing out", strerror(errno));
        exit(1);
    }
    return (retval);
}


int safe_system_timeout(const char *cmd, char * file,int timeout) 
{
    int ret = 0;
    pid_t pid;
    int status;
    time_t now;
    struct stat buf;
    char snmpfile[1024];  


    if(cmd == NULL)
        return -1;

    if(timeout< 0){
        return -2;
    }

    now = time(NULL);
    if((pid = vfork()) < 0){
        return -1;
    }else if(pid == 0){
        execl("/bin/bash", "sh", "-c", cmd, (char *)0);
        _exit(127);
    }else{
        while(time(NULL) - now <= timeout){
            //判断文件是否更新
            ret =stat( file, &buf );
            if(ret==0  && buf.st_mtim.tv_sec >= now ){
                return 0;
            }
            sleep(1);
        }
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        return -3;
    }
}



/*=====================================================================
* ABST. : 调用安全调用Linux 命令函数
* FUNC. : safe_system
* ARGS  : char* command
*         
* RETURN: 是否异常
* DESC. : 
* AUTHOR: wangkun
* DATE  : 2016-10-12
判断一个system函数调用shell脚本是否正常结束的方法应该是如下3个条件同时成立：
（1）-1 != status
（2）WIFEXITED(status)为真
（3）0 == WEXITSTATUS(status)
=======================================================================*/
int safe_system(char* command)  
{  
    pid_t status;  
	status = system(command);  
    if (-1 == status)  {
	
        debug(LOG_DEBUG,"system error!\n");

		return -1;
    }  
    else {
	
        //debug(LOG_DEBUG,"exit status value = [0x%x]\n", status); 
	
        if (WIFEXITED(status))  {  
            if (0 == WEXITSTATUS(status))  {
				
                //debug(LOG_DEBUG,"run shell command successfully.\n");

				return 1; 
            }  
            else {
				
                debug(LOG_DEBUG,"run shell command, command exit code: %d\n", WEXITSTATUS(status));
				
				return -1;				
            }  
        }  
        else  {
			
            debug(LOG_DEBUG,"exit status = [%d]\n", WEXITSTATUS(status));

			return -1;
        }  
    }  
     
}  


int safe_write(int fd,void *buffer,int length)
{
	int bytes_left;
	int written_bytes;
	char *ptr;

	ptr=buffer;
	bytes_left=length;
	while(bytes_left>0){
		written_bytes=write(fd,ptr,bytes_left);
		
		if(written_bytes<=0){       
                 if(errno == EINTR || errno == EAGAIN){ // || errno == EWOULDBLOCK || errno == EAGAIN
					 written_bytes=0;
				 }
                 else	
				 {
					debug(LOG_DEBUG,"errno=%d:%s\n",errno,strerror(errno));
					return (-1);
				 }
         }
        bytes_left-=written_bytes;
        ptr+=written_bytes;     
	}
return(0);
}

int safe_read(int fd,void *buffer,int length)
{
	int bytes_left;
	int bytes_read=0;
	char *ptr=buffer;
	bytes_left=length; 
	while(bytes_left>0 ){
			bytes_read=read(fd,ptr,bytes_left);	
			if(bytes_read<0){
				debug(LOG_DEBUG,"errno=%d\n",errno);
				debug(LOG_DEBUG,"error=%s\n",strerror(errno));
			  if(errno == EINTR ) //|| errno == EWOULDBLOCK || errno == EAGAIN
				  bytes_read=0;
			  else if(errno=EAGAIN) {bytes_read=0;continue;}
			  else  return(-1);
			}
			else if(bytes_read==0)  break;
			bytes_left-=bytes_read;
			ptr+=bytes_read;
		
	}
 return(length-bytes_left);
}





