int trave_dir(char* path, char p[][MAX_FILENAME_LENTH],int *filenum  ) //char filename[][MAX_FILENAME_LENTH]
{
    DIR *d; //����һ�����
    struct dirent *file; //readdir�����ķ���ֵ�ʹ��������ṹ����
    struct stat sb;    
    int len=0;
    if(!(d = opendir(path))){
        printf("error opendir %s!!!\n",path);
        return -1;
    }
    while((file = readdir(d)) != NULL){
        //�ѵ�ǰĿ¼.����һ��Ŀ¼..�������ļ���ȥ����������ѭ������Ŀ¼
        if(strncmp(file->d_name, ".", 1) == 0)
            continue;
        strcpy(p, file->d_name); //������������ļ���
		printf("p[%d]=%s\n",len,p);
		len++;
		p++;
        //�жϸ��ļ��Ƿ���Ŀ¼�����Ƿ������������㣬�����Ҷ���ֻ����������Ŀ¼��̫��Ͳ����ˣ�ʡ���ѳ�̫���ļ�
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