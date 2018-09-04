int trave_dir(char* path, char p[][MAX_FILENAME_LENTH],int *filenum  ) //char filename[][MAX_FILENAME_LENTH]
{
    DIR *d; //声明一个句柄
    struct dirent *file; //readdir函数的返回值就存放在这个结构体中
    struct stat sb;    
    int len=0;
    if(!(d = opendir(path))){
        printf("error opendir %s!!!\n",path);
        return -1;
    }
    while((file = readdir(d)) != NULL){
        //把当前目录.，上一级目录..及隐藏文件都去掉，避免死循环遍历目录
        if(strncmp(file->d_name, ".", 1) == 0)
            continue;
        strcpy(p, file->d_name); //保存遍历到的文件名
		printf("p[%d]=%s\n",len,p);
		len++;
		p++;
        //判断该文件是否是目录，及是否已搜索了三层，这里我定义只搜索了三层目录，太深就不搜了，省得搜出太多文件
        //if(stat(file->d_name, &sb) >= 0 && S_ISDIR(sb.st_mode) && depth <= 3){
        //    trave_dir(file->d_name, depth + 1,filename,filenum);
        //}
		//if(len >= MAX_FILE_NUM ) break;	
    }
	*filenum=len;
    closedir(d);
    return 0;
}

int get_dir_filenum(char* path )
{
    DIR *d; //声明一个句柄
    struct dirent *file; //readdir函数的返回值就存放在这个结构体中 
    int FileNum=0;
    if(!(d = opendir(path))){
        printf("error opendir %s!!!\n",path);
        return -1;
    }
    while((file = readdir(d)) != NULL){
        //把当前目录.，上一级目录..及隐藏文件都去掉，避免死循环遍历目录
        if(strncmp(file->d_name, ".", 1) == 0)
            continue;
        FileNum++;
    }
    closedir(d);
    return FileNum;
}