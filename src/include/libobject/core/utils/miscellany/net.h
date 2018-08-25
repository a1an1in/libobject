#ifndef __NET_H__
#define __NET_H__

#include <sys/types.h>

int get_mac(char *ifname,unsigned char *mac, int len);
int get_local_ip(char *ifname,char *ipaddr);
int set_local_ip(char *ifname,const char *ipaddr);  
int get_local_netmask(char *ifname,char *netmask_addr);  
int set_local_netmask(char *ifname,char *netmask_addr);  
int inet_is_in_same_subnet(char *ip_str,char *net_if_name);
int get_broadcast_addr(char *ifname, char *ipaddr);

#endif

