/* vim: set et sw=4 ts=4 sts=4 : */
/********************************************************************\
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
 \********************************************************************/

/* $Id$ */
/** @file conf.c
  @brief Config file parsing
  @author Copyright (C) 2004 Philippe April <papril777@yahoo.com>
  @author Copyright (C) 2007 Benoit Grégoire, Technologies Coeus inc.
 */

//#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <pthread.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <dirent.h>

#include "safe.h"
#include "config.h"
#include "debug.h"
#include "linkedlist.h"
#include "unit.h"


static void
parse_local_monitor_mac(const char *ptr);
static void
parse_datatype(const char *ptr);
static void
parse_local_phone_number(const char *ptr);


/** @internal
 * Holds the current configuration of the gateway */
static s_config config;

/**
 * Mutex for the configuration file, used by the auth_servers related
 * functions. */
pthread_mutex_t config_mutex = PTHREAD_MUTEX_INITIALIZER;

/** @internal
 * A flag.  If set to 1, there are missing or empty mandatory parameters in the config
 */
static int missing_parms;

/** @internal
 The different configuration options */
typedef enum {
	oBadOption,
    oDaemon,
    oLogFile,
    oDebugLevel,
	oAppFile,
	oDeviceMac,
	oDeviceId,
	oMonitorMenmac,
	oDevicePhone,
	oDataType,
	oUpdateTime,
	oServerUrl,
	oSrcPath,
} OpCodes;

/** @internal
 The config file keywords for the different configuration options */
static const struct {
    const char *name;
    OpCodes opcode;
} keywords[] = {
	{
    "daemon", oDaemon}, {
    "logfile", oLogFile}, {
    "debuglevel", oDebugLevel}, {
    "appfile", oAppFile},{
	"devicemac",oDeviceMac},{
	"deviceid",oDeviceId},{
	"monitormenmac",oMonitorMenmac},{
	"devicephone",oDevicePhone},{
	"datatype",	oDataType},{
	"updatetime",oUpdateTime},{
	"serverurl",oServerUrl},{
	"srcpath",oSrcPath},
	};

static void config_notnull(const void *, const char *);
static int parse_boolean_value(char *);


/** Accessor for the current gateway configuration
@return:  A pointer to the current config.  The pointer isn't opaque, but should be treated as READ-ONLY
 */
s_config *
config_get_config(void)
{
    return &config;
}

/** Sets the default config parameters and initialises the configuration system */
void
config_init(void)
{

    debug(LOG_DEBUG, "Setting default config parameters");
    config.configfile = safe_strdup(DEFAULT_CONFIGFILE);
    debugconf.log_stderr = 1;
    config.appfile=NULL;
	config.updatetime=60;
	config.devicemac=NULL;
	config.deviceid=NULL;
	config.serverurl=NULL;
	config.srcpath=NULL;
	config.P_t_Monitor_mac=NULL;
	config.P_t_Monitor_server_mac=NULL;
	config.P_t_Phonenum=NULL;
	config.P_t_server_Phonenum=NULL;
	config.P_t_Datasrctype=NULL;
}

/** @internal
Parses a single token from the config file
*/
static OpCodes
config_parse_token(const char *cp, const char *filename, int linenum)
{
    int i;

    for (i = 0; keywords[i].name; i++)
        if (strcasecmp(cp, keywords[i].name) == 0)
            return keywords[i].opcode;

    debug(LOG_ERR, "%s: line %d: Bad configuration option: %s", filename, linenum, cp);
    return oBadOption;
}


/**
@param filename Full path of the configuration file to be read
*/
void
config_read(const char *filename)
{
    FILE *fd;
    char line[MAX_BUF], *s, *p1, *p2, *rawarg = NULL;
    int linenum = 0, opcode, value;
    size_t len;

    debug(LOG_DEBUG, "Reading configuration file '%s'", filename);

    if (!(fd = fopen(filename, "r"))) {
        debug(LOG_ERR, "Could not open configuration file '%s', " "exiting...", filename);
        exit(1);
    }

    while (!feof(fd) && fgets(line, MAX_BUF, fd)) {
        linenum++;
        s = line;

        if (s[strlen(s) - 1] == '\n')
            s[strlen(s) - 1] = '\0';

        if ((p1 = strchr(s, ' '))) {
            p1[0] = '\0';
        } else if ((p1 = strchr(s, '\t'))) {
            p1[0] = '\0';
        }

        if (p1) {
            p1++;

            // Trim leading spaces
            len = strlen(p1);
            while (*p1 && len) {
                if (*p1 == ' ')
                    p1++;
                else
                    break;
                len = strlen(p1);
            }
            rawarg = safe_strdup(p1);
            if ((p2 = strchr(p1, ' '))) {
                p2[0] = '\0';
            } else if ((p2 = strstr(p1, "\r\n"))) {
                p2[0] = '\0';
            } else if ((p2 = strchr(p1, '\n'))) {
                p2[0] = '\0';
            }
        }

        if (p1 && p1[0] != '\0') {
            /* Strip trailing spaces */

            if ((strncmp(s, "#", 1)) != 0) {
                debug(LOG_DEBUG, "Parsing token: %s, " "value: %s", s, p1);
                opcode = config_parse_token(s, filename, linenum);

                switch (opcode) {

                case oDaemon:
                    if (config.daemon == -1 && ((value = parse_boolean_value(p1)) != -1)) {
                        config.daemon = value;
                        if (config.daemon > 0) {
                            debugconf.log_stderr = 0;
                        } else {
                            debugconf.log_stderr = 1;
                        }
                    }
                    break;
                case oLogFile:
                    config.logfile = safe_strdup(p1);
                    break;
                case oDebugLevel:
                    sscanf(p1, "%d", &config.debuglevel);
                    debugconf.debuglevel = config.debuglevel;
                    break;
                case oAppFile:
                    config.appfile = safe_strdup(p1);
                    break;
				case oUpdateTime:
                    sscanf(p1, "%d", &config.updatetime);
                    break;
				case oDeviceMac:
                    config.devicemac = safe_strdup(p1);
                    break;
				case oDeviceId:
					config.deviceid = safe_strdup(p1);
					break;
				case oMonitorMenmac:
					parse_local_monitor_mac(p1);
					break;
				case oDevicePhone:
					parse_local_phone_number(p1);
					break;
				case oDataType:
					parse_datatype(p1);
					break;
				case oServerUrl:
					config.serverurl = safe_strdup(p1);
					break;
				case oSrcPath:
					config.srcpath = safe_strdup(p1);
					break;
                default:
                    debug(LOG_ERR, "Bad option on line %d " "in %s.", linenum, filename);
                    //debug(LOG_ERR, "Exiting...");
                    //exit(-1);
                    break;
                }
            }
        }
        if (rawarg) {
            free(rawarg);
            rawarg = NULL;
        }
    }

    fclose(fd);
}

/** @internal
Parses a boolean value from the config file
*/
static int
parse_boolean_value(char *line)
{
    if (strcasecmp(line, "yes") == 0) {
        return 1;
    }
    if (strcasecmp(line, "no") == 0) {
        return 0;
    }
    if (strcmp(line, "1") == 0) {
        return 1;
    }
    if (strcmp(line, "0") == 0) {
        return 0;
    }

    return -1;
}







/** @internal
    Verifies that a required parameter is not a null pointer
*/
static void
config_notnull(const void *parm, const char *parmname)
{
    if (parm == NULL) {
        debug(LOG_ERR, "%s is not set", parmname);
        missing_parms = 1;
    }
}


/** @internal
 * Add a string to the T_String_list list. It prepends for simplicity.
 * @param string  to add T_String_list.

static void
add_string_to_list(const char *string)
{
    T_String_list *p = NULL;

    p = (T_String_list *)safe_malloc(sizeof(T_String_list));
    p->str = safe_strdup(string);

    if (config.p_t_str == NULL) {
        p->next = NULL;
        config.p_t_str = p;
    } else {
        p->next = config.p_t_str;
        config.p_t_str = p;
    }
}
*/





/** @internal
 * Parse the string list deamon .
 */
static void
parse_local_monitor_mac(const char *ptr)
{

	char *ptrcopy = NULL;
    char *p_str = NULL;
    char *tmp=NULL;
	t_Monitor_mac *p = NULL;
    debug(LOG_DEBUG, "Parsing string [%s] for deamon", ptr);

    /* strsep modifies original, so let's make a copy */
    ptrcopy = safe_strdup(ptr);

    while ((p_str = strsep(&ptrcopy, ","))) {
	        /* Skip leading spaces. */
        while (*p_str != '\0' && isblank(*p_str)) {
            p_str++;
        }
        if (*p_str == '\0') {  /* Equivalent to strcmp(p_str, "") == 0 */
            continue;
        }
        /* Remove any trailing blanks. */
        tmp = p_str;
        while (*tmp != '\0' && !isblank(*tmp)) {
            tmp++;
        }
        if (*tmp != '\0' && isblank(*tmp)) {
            *tmp = '\0';
        }

        debug(LOG_DEBUG, "Adding string list [%s] to T_Monitor_mac", p_str);
		if (is_valid_mac_addr(p_str)!=0){
			continue;
		}

	    p = (t_Monitor_mac *)safe_malloc(sizeof(t_Monitor_mac));
	    p->plocal_mac = safe_strdup(p_str);

	    if (config.P_t_Monitor_mac == NULL) {
			config.P_t_Monitor_mac = safe_malloc(sizeof(t_Monitor_mac));
			p->next = NULL;
	        config.P_t_Monitor_mac = p;
	    } else {
	        p->next = config.P_t_Monitor_mac;
	        config.P_t_Monitor_mac = p;
	    }


	}

	config.P_t_Monitor_mac=sort_local_mac_list(config.P_t_Monitor_mac);
    for(p=config.P_t_Monitor_mac;p!=NULL;p=p->next){
		debug(LOG_DEBUG, "local_mac=%s\n",p->plocal_mac);
    }

	free(ptrcopy);
}

/** @internal
 * Parse the datatype of MAC WIFIDOG DPI .
 */
static void
parse_datatype(const char *ptr)
{

	char *ptrcopy = NULL;
    char *p_str = NULL;
    char *tmp=NULL;
	t_Datasrctype *p = NULL;
    debug(LOG_DEBUG, "Parsing string [%s] for deamon", ptr);

    /* strsep modifies original, so let's make a copy */
    ptrcopy = safe_strdup(ptr);

    while ((p_str = strsep(&ptrcopy, ","))) {
	        /* Skip leading spaces. */
        while (*p_str != '\0' && isblank(*p_str)) {
            p_str++;
        }
        if (*p_str == '\0') {  /* Equivalent to strcmp(p_str, "") == 0 */
            continue;
        }
        /* Remove any trailing blanks. */
        tmp = p_str;
        while (*tmp != '\0' && !isblank(*tmp)) {
            tmp++;
        }
        if (*tmp != '\0' && isblank(*tmp)) {
            *tmp = '\0';
        }

        debug(LOG_DEBUG, "Adding string list [%s] to t_Datasrctype", p_str);

	    p = (t_Datasrctype *)safe_malloc(sizeof(t_Datasrctype));
	    p->pdatatype = safe_strdup(p_str);

	    if (config.P_t_Datasrctype == NULL) {
			config.P_t_Datasrctype = safe_malloc(sizeof(t_Datasrctype));
			p->next = NULL;
	        config.P_t_Datasrctype = p;
	    } else {
	        p->next = config.P_t_Datasrctype;
	        config.P_t_Datasrctype = p;
	    }


	}

	free(ptrcopy);
}


/** @internal
 * Parse the phone_number .
 */
static void
parse_local_phone_number(const char *ptr)
{

	char *ptrcopy = NULL;
    char *p_str = NULL;
    char *tmp=NULL;
	t_Phonenum *p = NULL;
    debug(LOG_DEBUG, "Parsing string [%s] for deamon", ptr);

    /* strsep modifies original, so let's make a copy */
    ptrcopy = safe_strdup(ptr);

    while ((p_str = strsep(&ptrcopy, ","))) {
	        /* Skip leading spaces. */
        while (*p_str != '\0' && isblank(*p_str)) {
            p_str++;
        }
        if (*p_str == '\0') {  /* Equivalent to strcmp(p_str, "") == 0 */
            continue;
        }
        /* Remove any trailing blanks. */
        tmp = p_str;
        while (*tmp != '\0' && !isblank(*tmp)) {
            tmp++;
        }
        if (*tmp != '\0' && isblank(*tmp)) {
            *tmp = '\0';
        }

        debug(LOG_DEBUG, "Adding string list [%s] to t_Datasrctype", p_str);

	    p = (t_Phonenum *)safe_malloc(sizeof(t_Phonenum));
	    p->plocal_phone = safe_strdup(p_str);

	    if (config.P_t_Phonenum == NULL) {
			config.P_t_Phonenum = safe_malloc(sizeof(t_Phonenum));
			p->next = NULL;
	        config.P_t_Phonenum = p;
	    } else {
	        p->next = config.P_t_Phonenum;
	        config.P_t_Phonenum = p;
	    }


	}

	free(ptrcopy);
}



/** @internal
    write logs to logfiles
*/
void
applog(char *buf)
{
    char timebuf[24];
    time_t t=time(NULL) ;
	//char *buffer;
	//int size_m;
    FILE *fd;
    if( config.appfile!=NULL && buf !=NULL ){
        memset(timebuf,0,sizeof(timebuf));
        fd = fopen(config.appfile, "a+");
        if(fd==NULL){
            debug(LOG_ERR,"open file %s fail",config.appfile);
        }else{
		   strftime(timebuf,sizeof(timebuf)-1,"%Y%m%d %H:%M:%S", localtime(&t));
           fprintf( fd, "%s:%s \n", timebuf,buf);
           fclose(fd);
		}
    }
}

