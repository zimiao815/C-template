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
/** @file conf.h
    @brief Config file parsing
    @author Copyright (C) 2004 Philippe April <papril777@yahoo.com>
*/

#ifndef CONFIG_H
#define CONFIG_H

#define MAX_BUF 512
#define DEFAULT_LOGLEVEL LOG_INFO
#define DEFAULT_CONFIGFILE "../a.conf"

/**
 * String list deamon structure
 */

typedef struct Monitor_mac_t{	
	char *plocal_mac;
	struct Monitor_mac_t *next;
}t_Monitor_mac;

typedef struct Monitor_server_mac_t{	
	char *pserver_mac;
	struct Monitor_server_mac_t *next;
}t_Monitor_server_mac;


typedef struct Phonenum_t{	
	char *plocal_phone;
	struct Phonenum_t *next;
}t_Phonenum;

typedef struct Server_Phonenum_t{	
	char *pserver_phone;
	struct Server_Phonenum_t *next;
}t_Server_Phonenum;



typedef struct datasrctype_t{	
	char *pdatatype;
	struct datasrctype_t *next;
	
}t_Datasrctype;



/**
 * Configuration structure
 */
typedef struct {
	
	int debuglevel;
	int log_stderr;
	int  daemon; 
	int  updatetime;
	char * appfile;
	char * logfile;
	char * configfile;
	char * devicemac;
	char * deviceid;
	char * serverurl;
	char * srcpath;
	t_Monitor_mac  *P_t_Monitor_mac;
	t_Monitor_server_mac  *P_t_Monitor_server_mac;
	t_Phonenum  *P_t_Phonenum;
	t_Server_Phonenum *P_t_server_Phonenum;
	t_Datasrctype *P_t_Datasrctype;
} s_config;



/** @brief Get the current gateway configuration */
s_config *config_get_config(void);

/** @brief Initialise the conf system */
void config_init(void);


/** @brief Reads the configuration file */
void config_read(const char *filename);



void applog(char *buf);
/*
#define logger(level, format...) _logger(__FILE__,__FUNCTION__, __LINE__, level, format)

#define LOCK_CONFIG() do { \
    debug(LOG_DEBUG, "Locking config"); \
    pthread_mutex_lock(&config_mutex); \
    debug(LOG_DEBUG, "Config locked"); \
} while (0)

#define UNLOCK_CONFIG() do { \
    debug(LOG_DEBUG, "Unlocking config"); \
    pthread_mutex_unlock(&config_mutex); \
    debug(LOG_DEBUG, "Config unlocked"); \
} while (0)
*/
#endif                          /* CONFIG_H */
