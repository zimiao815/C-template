#ifndef _SAFE_H_
#define _SAFE_H_

#include <stdarg.h>             /* For va_list */
#include <sys/types.h>          /* For fork */
#include <unistd.h>             /* For fork */



/** @brief Safe version of malloc */
void *safe_malloc(size_t);

/** @brief Safe version of realloc */
void *safe_realloc(void *, size_t);


/* @brief Safe version of strdup */
char *safe_strdup(const char *);

/* @brief Safe version of asprintf */
int safe_asprintf(char **, const char *, ...);

/* @brief Safe version of vasprintf */
int safe_vasprintf(char **, const char *, va_list);

/* @brief Safe version of fork */
//pid_t safe_fork(void);

/* @brief Safe version of system with shell timeout killed */
int safe_system_timeout(const char *cmd, char * file,int timeout);

#endif                          /* _SAFE_H_ */
