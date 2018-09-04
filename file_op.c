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

/** @file file_op.c
    @brief Debug output routines
    @author Copyright (C) 2004 Philippe April <papril777@yahoo.com>
*/

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
#include <string.h>


#include "debug.h"
#include "file_op.h"
#include "safe.h"

t_dir_filename Fname_List;
t_filemac Fmac_list;



t_dir_filename *
get_dir_filename(void)
{
    return &Fname_List;
}

void
dir_filename_init(t_dir_filename * dfn)
{
  dfn->filestotal=0;
  dfn->pfnames=NULL;

  return;
}


t_filemac *
get_filemac_list(){
   return &Fmac_list;
}


void filemac_init(t_filemac * fm)
{
  fm->flag=0;
  memset(fm->pmac,0,sizeof(fm->pmac));
  memset(fm->timest,0,sizeof(fm->timest));

  return ;
}




int modify_file_content(char * files,int offset,char * value)
{

  FILE * pFile;
  char data[1024];
  pFile = fopen (files, "r+" );//创建一个只读的example.txt
  fseek ( pFile , offset , SEEK_SET ); //设置文件内部位置指针从文件的开始处偏移offset个字节
  fputs ( value , pFile ); //在新的文件内部指针处写下sam，
  fclose ( pFile );
  return 0;

}


int modify_file_assign_content(char * files,char s_c,char d_c )
{

  FILE * pFile;
  int c;
  int offset;
  pFile = fopen (files, "r+" );//创建一个只读的example.txt

  do
  {
     offset = ftell(pFile);
	 c = fgetc(pFile);

     if( feof(pFile) ){
          break ;
     }
	 if (c==s_c){
		fseek(pFile,-1,SEEK_CUR);
		fputc(d_c,pFile);
		offset = ftell(pFile);

		fseek(pFile,-1,SEEK_CUR);
		offset = ftell(pFile);

	 }
   }while(1);
  fclose ( pFile );
  return 0;

}




/*
 *rev_buf:从文件中的读取的所有内容
 *rev_data_len：从文件中的读取的所有内容的长度
 *注:处理文件内容以后，需要free(rev_buf)
 */

int  read_file_info(char *path ,char *file,char **rev_buf,int * rev_data_len)
{

    FILE *fp = NULL;
    DIR *dirptr=NULL;
    char *p;
    char path_file[1024];
    int size;
    memset(path_file,0,sizeof(path_file));
    if((dirptr=opendir(path))==NULL){
        mkdir(path,0775);
    }else{
        closedir(dirptr);
    }
    sprintf(path_file,"%s/%s",path,file);
	fp = fopen(path_file,"rb");
	if(NULL == fp){
		debug(LOG_ERR, "Error:Open %s file fail!\n", path_file);
		return -1;
	}
	//求得文件的大小
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	rewind(fp);
	//申请一块能装下整个文件的空间
	p = (char*)malloc(sizeof(char)*size);
	if(p==NULL){
		fclose(fp);
		return -1;
	}
	memset(p,0,sizeof(char)*size);
	//读文件
	fread(p,1,size,fp);//每次读一个，共读size次

	*rev_buf=p;
	*rev_data_len=size;
	fclose(fp);
	return 0;

}




int file_copy(char * src_file,char * des_file )
{
    FILE *fin,*fout;
    char buff[1024];
    int len;
    fin = fopen(src_file,"r+");  //r+ 打开可读写的文件，该文件必须存在。
    fout = fopen(des_file,"w+"); //w+ 打开可读写文件，若文件存在则文件长度清为零，即该文件内容会消失。若文件不存在则建立。
    if(NULL == fin || NULL == fout ){
		debug(LOG_ERR, "Error:Open %s or %s  fail!\n", src_file,des_file);
		return -1;
	}
    while(len = fread(buff,1,sizeof(buff),fin))
    {
        fwrite(buff,1,len,fout);
    }
    fclose(fin);
    fclose(fout);
    return 0;
}



int file_delete(char *src_file )
{
	if((access(src_file,F_OK))!=-1)
    {
       if( remove(src_file) == 0 ){
          //debug(LOG_ERR, "Error:Removed %s  fail!\n", src_file);
	   }else{
	    perror("remove");
		//debug(LOG_ERR, "Error:Removed %s  fail!\n", src_file);
	   }
    }
    else
    {
      debug(LOG_ERR, "Error:%s does not exist!\n", src_file);
    }

}






/********************************* Ŀ¼�ļ�����************************************/

/**
* @brief �����ļ��µ������ļ��������档
*
* @param [in] path Ŀ¼����
*        note:��Ҫ������·��,��β����б�� ����/home/mypath
* @param [out] fn ִ�б�����ļ����ƺ��ļ�������
* @return 0
*/


int trave_dir(char* path, t_dir_filename *fn  )
{
    DIR *dir; //����һ�����
    struct dirent *ptr; //readdir�����ķ���ֵ�ʹ��������ṹ����
	t_dir_filenames *p;

	char path_fliename[512];

    if(!(dir = opendir(path))){
        debug(LOG_ERR, "%s: is opened error\n", path);
		perror("perror");
        return -1;
    }

    while ((ptr=readdir(dir)) != NULL)
    {


		memset(path_fliename,0,sizeof(path_fliename));
		sprintf(path_fliename,"%s%s%s",path,"/",ptr->d_name);

		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)    ///current dir OR parrent dir
            continue;
        else if(ptr->d_type == 8){    ///file
		    p = (t_dir_filenames *)safe_malloc(sizeof(t_dir_filenames));
	        p->pfilename = safe_strdup(path_fliename);
	        if (fn->pfnames== NULL) {
			    fn->pfnames = (t_dir_filenames *) safe_malloc(sizeof(t_dir_filenames));
			    fn->pfnames->next= NULL;
	            fn->pfnames->pfilename = p->pfilename;
	        } else {
	             p->next = fn->pfnames;
	             fn->pfnames = p;
	        }
		    fn->filestotal++;

		}else if(ptr->d_type == 10){    ///link file
		    debug(LOG_DEBUG, "[%s] is a link file",path_fliename);
		}
        else if(ptr->d_type == 4)    ///dir
        {
			trave_dir(path_fliename, fn);
        }
    }
    closedir(dir);

    return 0;
}



void free_dir_file_list(void)
{
	t_dir_filenames *llll;
    t_dir_filename * p_s_dir_filename=get_dir_filename();

	for(llll=p_s_dir_filename->pfnames;llll!=NULL;llll=llll->next){
		debug(LOG_ERR, "substring=%s\n",llll->pfilename);
		free(llll->pfilename);
		llll->pfilename=NULL;
		free(llll);
	}
}





int get_dir_filenum(char* path )
{
    DIR *d; //����һ�����
    struct dirent *file; //readdir�����ķ���ֵ�ʹ��������ṹ����
    int FileNum=0;
    if(!(d = opendir(path))){
        printf("error opendir %s!!!\n",path);
        return -1;
    }
    while((file = readdir(d)) != NULL){
        //�ѵ�ǰĿ¼.����һ��Ŀ¼..�������ļ���ȥ����������ѭ������Ŀ¼
        if(strncmp(file->d_name, ".", 1) == 0)
            continue;
        FileNum++;
    }
    closedir(d);
    return FileNum;
}





/** @internal
 * Parse the
    char *col ��\tΪ�ָ�������,"1|3"��ʾ��ȡ��1 3�� ֻ����д2��
              ǰһ����ʾmac���ڵ�λ�ã���һ����ʾʱ������ڵ�λ��
              ��1��ʼ
 */
t_filemac *
parse_extract_found_mac(const char *ptr,char *col)
{

	char *ptrcopy = NULL;
	char *colcopy = NULL;
    char *p_str = NULL;
    char *tmp=NULL;
	int col_mac,col_timest,i=0,j=0;
    char timese[11];

	t_filemac *p=(t_filemac *)safe_malloc(sizeof(t_filemac)); //���ñ������ĺ�������free
	filemac_init(p);

    debug(LOG_DEBUG, "Parsing string [%s] for ptr deamon", ptr);
    debug(LOG_DEBUG, "Parsing string [%s] for col deamon", col);

    /* strsep modifies original, so let's make a copy */

    colcopy = safe_strdup(col);

    while ((p_str = strsep(&colcopy, "|"))) {

        debug(LOG_DEBUG, "parse_extract_found_mac [%s] ", p_str);
		if(i==0){
             col_mac=atoi(p_str);
		     i++;
		}else if ( i == 1 ){
			col_timest=atoi(p_str);
			i=0;
		}
	}


    ptrcopy = safe_strdup(ptr);

    while ((p_str = strsep(&ptrcopy, "\t"))) {

		j++;
		if(j==col_mac){
            memcpy(p->pmac,p_str,strlen(p_str));
			debug(LOG_DEBUG, "Parsing string [%s] for p->pmac", p->pmac);

		}else if(j==col_timest){
			 memcpy(p->timest,p_str,10);
			 debug(LOG_DEBUG, "Parsing string [%s] for p->timest", p->timest);
		}
	}

	free(ptrcopy);
	free(colcopy);

	return p;

}






