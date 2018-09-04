#include <stdio.h>  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <arpa/inet.h>
#include <string.h>  
#include <unistd.h>  
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
  
#include "sock.h"

//#define BUFSIZ 512

void u_alarm_handler()
{
	printf("alarm hander\n");
	
}




int tcp_server()  
{  
    int server_sockfd;//服务器端套接字  
    int client_sockfd;//客户端套接字  
    int len;  
    struct sockaddr_in my_addr;   //服务器网络地址结构体  
    struct sockaddr_in remote_addr; //客户端网络地址结构体  
    int sin_size;  
    char buf[BUFSIZ];  //数据传送的缓冲区
	
    memset(&my_addr,0,sizeof(my_addr)); //数据初始化--清零  
    my_addr.sin_family=AF_INET; //设置为IP通信  
    my_addr.sin_addr.s_addr=INADDR_ANY;//服务器IP地址--允许连接到所有本地地址上  
    my_addr.sin_port=htons(5555); //服务器端口号  
    sigset(SIGALRM, u_alarm_handler);  
    /*创建服务器端套接字--IPv4协议，面向连接通信，TCP协议*/  
    if((server_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)  
    {    
        perror("socket");  
        return 1;  
    }  
    int   flags   =   fcntl(server_sockfd,   F_GETFL,   0); 
    fcntl(server_sockfd,   F_SETFL,   flags   |   O_NONBLOCK);
        /*将套接字绑定到服务器的网络地址上*/  
    if (bind(server_sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))<0)  
    {  
        perror("bind");  
        return 1;  
    }  
      
    /*监听连接请求--监听队列长度为5*/  
    listen(server_sockfd,5);  
      
    sin_size=sizeof(struct sockaddr_in);  
      
    /*等待客户端连接请求到达*/  
    while((client_sockfd=accept(server_sockfd,(struct sockaddr *)&remote_addr,&sin_size))<0);  
//    {  
//        perror("accept");  
//       return 1;  
//    }  
    printf("accept client %s\n",inet_ntoa(remote_addr.sin_addr));  
    len=send(client_sockfd,"Welcome to my server\n",21,0);//发送欢迎信息  
      
    /*接收客户端的数据并将其发送给客户端--recv返回接收到的字节数，send返回发送的字节数*/  
	
	
    while((len=recv(client_sockfd,buf,BUFSIZ,0))>0)  
    {  
        buf[len]='\0';  
        printf("buf:%s\n",buf); 

        if(send(client_sockfd,buf,len,0)<0)  
        {  
            perror("write");  
            return 1;  
        }  
    }  
    close(client_sockfd);  
    close(server_sockfd);  
    return 0;  
}  




int tcp_client()  
{  
    int client_sockfd;  
    int len;  
    struct sockaddr_in remote_addr; //服务器端网络地址结构体  
    char buf[BUFSIZ];  //数据传送的缓冲区  

    memset(&remote_addr,0,sizeof(remote_addr)); //数据初始化--清零  
    remote_addr.sin_family=AF_INET; //设置为IP通信  
    remote_addr.sin_addr.s_addr=inet_addr("127.0.0.1");//服务器IP地址  
    remote_addr.sin_port=htons(5555); //服务器端口号  
      
    /*创建客户端套接字--IPv4协议，面向连接通信，TCP协议*/  
    if((client_sockfd=socket(PF_INET,SOCK_STREAM,0))<0)  
    {  
        perror("socket");  
        return 1;  
    }  
      
    /*将套接字绑定到服务器的网络地址上*/  
    if(connect(client_sockfd,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr))<0)  
    {  
        perror("connect");  
        return 1;  
    }  
    printf("connected to server\n");  
    len=recv(client_sockfd,buf,BUFSIZ,0);//接收服务器端信息  
    buf[len]='\0';  
    printf("%s",buf); //打印服务器端信息  
      
    /*循环的发送接收信息并打印接收信息--recv返回接收到的字节数，send返回发送的字节数*/  
    while(1)  
    {  
        printf("Enter string to send:");  
        scanf("%s",buf);  
        if(!strcmp(buf,"quit"))  
            break;  
        len=send(client_sockfd,buf,strlen(buf),0);  
        len=recv(client_sockfd,buf,BUFSIZ,0);  
        buf[len]='\0';  
        printf("received:%s\n",buf);  
    }  
    close(client_sockfd);//关闭套接字  
    return 0;  
}  


int udp_server()  
{  
    int server_sockfd;  
    int len;  
    struct sockaddr_in my_addr;   //服务器网络地址结构体  
         struct sockaddr_in remote_addr; //客户端网络地址结构体  
    int sin_size;  
    char buf[BUFSIZ];  //数据传送的缓冲区  
    memset(&my_addr,0,sizeof(my_addr)); //数据初始化--清零  
    my_addr.sin_family=AF_INET; //设置为IP通信  
    my_addr.sin_addr.s_addr=INADDR_ANY;//服务器IP地址--允许连接到所有本地地址上  
    my_addr.sin_port=htons(8000); //服务器端口号  
      
    /*创建服务器端套接字--IPv4协议，面向无连接通信，UDP协议*/  
    if((server_sockfd=socket(PF_INET,SOCK_DGRAM,0))<0)  
    {    
        perror("socket");  
        return 1;  
    }  
   
        /*将套接字绑定到服务器的网络地址上*/  
    if (bind(server_sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr))<0)  
    {  
        perror("bind");  
        return 1;  
    }  
    sin_size=sizeof(struct sockaddr_in);  
    printf("waiting for a packet.../n");  
      
    /*接收客户端的数据并将其发送给客户端--recvfrom是无连接的*/  
    if((len=recvfrom(server_sockfd,buf,BUFSIZ,0,(struct sockaddr *)&remote_addr,&sin_size))<0)  
    {  
        perror("recvfrom");   
        return 1;  
    }  
    printf("received packet from %s:/n",inet_ntoa(remote_addr.sin_addr));  
    buf[len]='\0';  
    printf("contents: %s\n",buf);  
    close(server_sockfd);  
        return 0;  
}  

int udp_client()  
{  
    int client_sockfd;  
    int len;  
        struct sockaddr_in remote_addr; //服务器端网络地址结构体  
    int sin_size;  
    char buf[BUFSIZ];  //数据传送的缓冲区  
    memset(&remote_addr,0,sizeof(remote_addr)); //数据初始化--清零  
    remote_addr.sin_family=AF_INET; //设置为IP通信  
    remote_addr.sin_addr.s_addr=inet_addr("127.0.0.1");//服务器IP地址  
    remote_addr.sin_port=htons(8000); //服务器端口号  
  
         /*创建客户端套接字--IPv4协议，面向无连接通信，UDP协议*/  
    if((client_sockfd=socket(PF_INET,SOCK_DGRAM,0))<0)  
    {    
        perror("socket");  
        return 1;  
    }  
    strcpy(buf,"This is a test message");  
    printf("sending: '%s'/n",buf);  
    sin_size=sizeof(struct sockaddr_in);  
      
    /*向服务器发送数据包*/  
    if((len=sendto(client_sockfd,buf,strlen(buf),0,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr)))<0)  
    {  
        perror("recvfrom");   
        return 1;  
    }  
    close(client_sockfd);  
    return 0;  
}  






