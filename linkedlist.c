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

#include "http.h"
#include "curl_op.h"


typedef struct Node  
{  
    int data;//数据域，用来存放数据域；  
    struct Node *pNext;//定义一个结构体指针，指向下一次个与当前节点数据类型相同的节点  
}NODE,*PNODE;  //NODE等价于 struct Node; PNODE等价于struct Node *； 此处用大写是为了与变量区分，可以让人容易变出是个数据类型  


void print_list(PNODE list)
{
  PNODE spf;
  spf=list;
  while (  spf->pNext != NULL ){
    printf("%d ",spf->data);
    spf=spf->pNext;
  }
  printf("%d\n",spf->data);
}


PNODE Create_List(int lenth)  
{  
    int len;  //存放链表的长度  
    int i;   //循环变量  
    int val;//用来临时存放用户输入的结点的值  
    PNODE List;  
    PNODE pHead=(PNODE)malloc(sizeof(NODE));//分配一个头节点  
    if(NULL==pHead)  
    {  
        printf("Memory allocation failure");  
        exit(-1);  
    }  
    else  
    {   PNODE pTail=pHead;  
        
        pHead->data=lenth;
        pHead->pNext=NULL;
           
         
        for(i=0;i<lenth;i++)  
        {  
            PNODE p=(PNODE)malloc(sizeof(NODE));  
            if(NULL==p)  
            {  
                printf("Memory allocation failure");  
                exit(-1);  
            }  
            else  
            {     
                printf("please input the value of list: ");  
                scanf("%d",&val);  
                p->data=val;  
                pTail->pNext=p;  
                p->pNext=NULL;  
                pTail=p;  
            }  
  
        }  
  
  
    }  
    return pHead;  
  
}  


//链表的第pos有效元素前面插入元素val，首先我们应该找到第pos个元素前面一个元素的位置；  
//当链表有3个元素时，pos=4，将不会进行插入操作  
int Insert_List(PNODE pHead,int pos,int val) 
{

   PNODE pIns;
   pIns=pHead;
   PNODE pNewnode;
   int i=0;
   while ( pIns != NULL && (i < pos-1) ){
     pIns=pIns->pNext;
     
     i++;  
 
   }
   if (pIns->pNext == NULL || i > pos-1 ){  //把链表为空的情况考虑进去了；i>pos-1 可以防止用户输入错误； 
     printf("Insert_List  failure, List  tail ");
     return 0;
   }   


   pNewnode=(PNODE)malloc( sizeof(NODE) );
   if(NULL==pNewnode){
        printf("Insert_List pNewnode failure");
        exit(-1);
   }
   pNewnode->data=val;   

   pNewnode->pNext=pIns->pNext;
   
   pIns->pNext=pNewnode;
   
   pHead->data++;

}


int Delete_List(PNODE pHead,int pos,int *val) 
{

  PNODE pDel,q;//q 指向要删除的节点
  pDel=pHead;
  
  int i=0;
  while ( pDel->pNext!=NULL && (i< pos-1)){
     pDel=pDel->pNext;
     i++;
  }
  if ( pDel->pNext==NULL || i > pos-1 );{
      printf("delete _node  no found!!!\n");
      return 0;
  }
  q=pDel->pNext;
  *val=q->data;
  pDel->pNext=q->pNext;
  free(q);
  q=NULL;////千万不可以忘记，否则会出现野指针；
  pHead->data--;

}



/*------------------------------链表排序-------------------------------*/

PNODE merge_list( PNODE head1,PNODE head2 )
{
    PNODE ptmp;
    if( head1==NULL ){
      return head2;
    }
    if(head2==NULL){
      return head1;
    }
    if ( head1->data <= head2->data ){
       ptmp=head1;
       ptmp->pNext=merge_list( head1->pNext,head2);
    }else{
       ptmp=head2;
       ptmp->pNext=merge_list( head1,head2->pNext);
    }

   return ptmp;

}




/*******************************************************************************************
第一个链表的头为head，第二个链表的头为慢指针的next指针。先排后面，在排前面。 
此时需要注意，链表要分成两份，从中间断开，因此需要将慢节点的next指针设置为NULL很关键！！！
********************************************************************************************/
PNODE sort_list(PNODE head)
{
   PNODE f; //fast slow
   PNODE s;
   PNODE p;
   if (head == NULL || head->pNext == NULL)//如果没有元素或者只有一个元素，那么就直接退出
      return head;
   f=head;
   s=head;
   while ( f->pNext!=NULL && f->pNext->pNext!=NULL ){ //当快指针到链表结尾时，慢指针刚好走到链表的中间节点。
      f=f->pNext->pNext;
      s=s->pNext;
   }
   if( f==head->pNext ){//如果此时只有两个节点，那么中间节点就是头节点，f=s=head->pNext，无限循环，因此特殊判断
     f=f->pNext;
     s->pNext=NULL;
     return merge_list(f,s);
   }
   p=s->pNext;
   f=sort_list(p);//排序后半部分
   s->pNext = NULL;//注意，一定要把前面从中间节点切断
   s=sort_list(head);//排序前面一部分
   
   PNODE temp = merge_list(s,f);//合并两个有序链表
   return temp;

}


/*------------------------------链表排序-------------------------------*/

t_Monitor_mac * merge_local_mac_list( t_Monitor_mac * head1,t_Monitor_mac * head2 )
{
    t_Monitor_mac * ptmp;
    if( head1==NULL ){
      return head2;
    }
    if(head2==NULL){
      return head1;
    }
    if ( strcmp(head1->plocal_mac, head2->plocal_mac)<=0 ){
       ptmp=head1;
       ptmp->next=merge_local_mac_list( head1->next,head2);
    }else{
       ptmp=head2;
       ptmp->next=merge_local_mac_list( head1,head2->next);
    }

   return ptmp;

}



/*******************************************************************************************
第一个链表的头为head，第二个链表的头为慢指针的next指针。先排后面，在排前面。 
此时需要注意，链表要分成两份，从中间断开，因此需要将慢节点的next指针设置为NULL很关键！！！
********************************************************************************************/
t_Monitor_mac * sort_local_mac_list(t_Monitor_mac * head)
{
   t_Monitor_mac * f; //fast slow
   t_Monitor_mac * s;
   t_Monitor_mac * p;
   t_Monitor_mac * temp;
   if (head == NULL || head->next == NULL)//如果没有元素或者只有一个元素，那么就直接退出
      return head;
   f=head;
   s=head;
   while ( f->next!=NULL && f->next->next!=NULL ){ //当快指针到链表结尾时，慢指针刚好走到链表的中间节点。
      f=f->next->next;
      s=s->next;
   }
   if( f==head->next ){//如果此时只有两个节点，那么中间节点就是头节点，f=s=head->pNext，无限循环，因此特殊判断
     f=f->next;
     s->next=NULL;
     return merge_local_mac_list(f,s);
   }
   p=s->next;
   f=sort_local_mac_list(p);//排序后半部分
   s->next = NULL;//注意，一定要把前面从中间节点切断
   s=sort_local_mac_list(head);//排序前面一部分
   
   temp = merge_local_mac_list(s,f);//合并两个有序链表
   return temp;

}




/**
* @brief �������Ľڵ�
*
* @param [in] pstart �����п�ʼ�ڵ�ָ��
*        
* @param [in] pend  �����н����ڵ�ָ��
* @return ���Ľڵ��ַ
*/



t_Monitor_mac *   local_FindMidNode(t_Monitor_mac *  pstart,t_Monitor_mac *pend )  
{  
    t_Monitor_mac *  fast = NULL ;  
    t_Monitor_mac *  slow = NULL;  
     if (pstart==NULL)
   	  return NULL;
  
    fast = slow = pstart;  
      
    while(fast != pend && fast->next != pend )  
    {  
        fast = fast->next ->next ;  
        slow = slow->next;  
    }  
    return slow;  
} 




/****************************************
ժҪ�����ֲ���
* @param [in] phead �����п�ʼ�ڵ�ָ��
*        
* @param [in] target  ��Ҫ���ҵ�Ŀ�괮
* @return Ŀ���ַ����Ľڵ��ַ

****************************************/

t_Monitor_mac * local_binseach(t_Monitor_mac *  phead,char * target)
{
   t_Monitor_mac *pmid=NULL;//��ǰ�����Ľڵ�
   t_Monitor_mac * pstart=NULL;//���β��ҵĿ�ʼ�Ľڵ�λ��
   t_Monitor_mac * pend=NULL;//���β��ҵĽ����Ľڵ�λ��
   pstart=phead;
   if (pstart==NULL)
   	  return NULL;
   while ( pstart!=pend ){
      pmid=local_FindMidNode(pstart,pend);
      if (pmid==NULL)
   	     return NULL;
      if ( strcmp(pmid->plocal_mac,target)<0 ){
          if(pstart!=pmid)
		     pstart=pmid;
          else
		  	break;
		 
      }else if(strcmp(pmid->plocal_mac,target)>0 ){
          if(pend!=pmid)
             pend=pmid;
		  else
		  	break;
        
      }else {
        return pmid;
      }
	  
   }
   return NULL;
}








/*------------------------------链表排序-------------------------------*/

t_Monitor_server_mac * merge_server_mac_list( t_Monitor_server_mac * head1,t_Monitor_server_mac * head2 )
{
    t_Monitor_server_mac * ptmp;
    if( head1==NULL ){
      return head2;
    }
    if(head2==NULL){
      return head1;
    }
    if ( strcmp(head1->pserver_mac , head2->pserver_mac)<=0 ){
       ptmp=head1;
       ptmp->next=merge_server_mac_list( head1->next,head2);
    }else{
       ptmp=head2;
       ptmp->next=merge_server_mac_list( head1,head2->next);
    }

   return ptmp;

}



/*******************************************************************************************
第一个链表的头为head，第二个链表的头为慢指针的next指针。先排后面，在排前面。 
此时需要注意，链表要分成两份，从中间断开，因此需要将慢节点的next指针设置为NULL很关键！！！
********************************************************************************************/
t_Monitor_server_mac * sort_server_mac_list(t_Monitor_server_mac * head)
{
   t_Monitor_server_mac * f; //fast slow
   t_Monitor_server_mac * s;
   t_Monitor_server_mac * p;
   t_Monitor_server_mac * temp;
   if (head == NULL || head->next == NULL)//如果没有元素或者只有一个元素，那么就直接退出
      return head;
   f=head;
   s=head;
   while ( f->next!=NULL && f->next->next!=NULL ){ //当快指针到链表结尾时，慢指针刚好走到链表的中间节点。
      f=f->next->next;
      s=s->next;
   }
   if( f==head->next ){//如果此时只有两个节点，那么中间节点就是头节点，f=s=head->pNext，无限循环，因此特殊判断
     f=f->next;
     s->next=NULL;
     return merge_server_mac_list(f,s);
   }
   p=s->next;
   f=sort_server_mac_list(p);//排序后半部分
   s->next = NULL;//注意，一定要把前面从中间节点切断
   s=sort_server_mac_list(head);//排序前面一部分
   
   temp = merge_server_mac_list(s,f);//合并两个有序链表
   return temp;

}



/**
* @brief �������Ľڵ�
*
* @param [in] pstart �����п�ʼ�ڵ�ָ��
*        
* @param [in] pend  �����н����ڵ�ָ��
* @return ���Ľڵ��ַ
*/



t_Monitor_server_mac *   server_FindMidNode(t_Monitor_server_mac *  pstart,t_Monitor_server_mac *pend )  
{  
    t_Monitor_server_mac *  fast = NULL ;  
    t_Monitor_server_mac *  slow = NULL;  
     if (pstart==NULL)
   	  return NULL;
  
    fast = slow = pstart;  
      
    while(fast != pend && fast->next!=pend )  
    {  
        fast = fast->next ->next ;  
        slow = slow->next;  
    }  
    return slow;  
} 




/****************************************
ժҪ�����ֲ���
* @param [in] phead �����п�ʼ�ڵ�ָ��
*        
* @param [in] target  ��Ҫ���ҵ�Ŀ�괮
* @return ���Ľڵ��ַ

****************************************/

t_Monitor_server_mac * server_binseach(t_Monitor_server_mac *  phead,char * target)
{
   t_Monitor_server_mac *pmid=NULL;//��ǰ�����Ľڵ�
   t_Monitor_server_mac * pstart=NULL;//���β��ҵĿ�ʼ�Ľڵ�λ��
   t_Monitor_server_mac * pend=NULL;//���β��ҵĽ����Ľڵ�λ��
   pstart=phead;
   if (pstart==NULL)
   	  return NULL;
   while ( pstart!=pend ){
      pmid=server_FindMidNode(pstart,pend);
      if (pmid==NULL)
   	     return NULL;
      if ( strcmp(pmid->pserver_mac,target)<0 ){
          if(pstart!=pmid)
		     pstart=pmid;//�˴�Ӧ��ʹ��pmid�ĺ�̽ڵ㸳ֵ���ã����㷨��Щ��ͬ,���������Ƚ��鷳����ʱ����д
          else
		  	break;
		 
      }else if(strcmp(pmid->pserver_mac,target)>0 ){
          if(pend!=pmid)
             pend=pmid;//�˴�Ӧ��ʹ��pmid��ǰ���ڵ㸳ֵ���ã����������Ƚ��鷳����ʱ����д
		  else
		  	break;
        
      }else {
        return pmid;
      }
	  
   }
   return NULL;
}







