/* vim: set et ts=4 sts=4 sw=4 : */
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

/** @file file_op.h
    @brief Debug output routines
    @author Copyright (C) 2004 Philippe April <papril777@yahoo.com>
*/

#ifndef _FILEOP_H_
#define _FILEOP_H_




/*
用于遍历目录里所有的文件
保存与此结构体的链表中
*/
typedef struct dir_filenames{
		  char * pfilename;
		  struct dir_filenames *next;
}t_dir_filenames;


typedef struct dir_filename_t{
      t_dir_filenames  *pfnames;
      int    filestotal;
}t_dir_filename;

typedef struct filemac_t{
      int    flag; // 1:服务器布控的mac，2本地布控的mac。用于判断调用执行不同的curl接口的判断
      char   timest[32];
	  char   pmac[20];
	  //struct filemac_t *next;
}t_filemac;




/* @brief modify file of offset to value */
int modify_file_content(char * files,int offset,char * value);
int modify_file_assign_content(char * files,char s_c,char d_c );
int read_file_info(char *path ,char *file,char **rev_buf,int * rev_data_len);
int file_copy(char *src_file,char * des_file );
int file_delete(char *src_file );

/***********************************************************/

void
filemac_init(t_filemac * fm);

t_dir_filename *
get_dir_filename(void);
void
dir_filename_init(t_dir_filename * dfn);

int
trave_dir(char* path, t_dir_filename *fn  );
void
free_dir_file_list(void);

t_filemac *
parse_extract_found_mac(const char *ptr,char *col);



#endif /* _FILEOP_H_ */
