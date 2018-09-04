#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

t_Monitor_mac * sort_local_mac_list(t_Monitor_mac * head);
t_Monitor_mac * local_binseach(t_Monitor_mac *  phead,char * target);
t_Monitor_server_mac * sort_server_mac_list(t_Monitor_server_mac * head);
t_Monitor_server_mac * server_binseach(t_Monitor_server_mac *  phead,char * target);


#endif                          /* _LINKEDLIST_H_ */