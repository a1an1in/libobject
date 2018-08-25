/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>     
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/miscellany/net.h>

void print_mac(unsigned char *mac)
{
    printf("%x:%x:%x:%x:%x:%x\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

int get_mac(char *ifname,unsigned char *mac, int len) 
{
    struct ifreq ifreq;
    int sock;

    if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror ("socket");
        return -1;
    }
    strcpy (ifreq.ifr_name, ifname); 

/*
 *    if (ioctl (sock, SIOCGIFHWADDR, &ifreq) < 0)
 *    {
 *        perror ("ioctl");
 *        return -1;
 *    }
 *
 *    memcpy(mac,ifreq.ifr_hwaddr.sa_data,len);
 */

    close(sock);

    return 1;
    
}

void print_ipaddr(char *ipaddr)
{
    printf("ipaddr:%s\n",ipaddr);
}

int get_local_ip_num(char *ifname)  
{  
    int sk;  
    struct   sockaddr_in *sin;  
    struct   ifreq ifreq;     

    if ((sk=socket(AF_INET, SOCK_STREAM, 0)) == -1)  
    {  
        printf("socket create failse...GetLocalIp!\n");  
        return -1;  
    }  

    memset(&ifreq, 0, sizeof(ifreq));     
    strncpy(ifreq.ifr_name, ifname, sizeof(ifreq.ifr_name) - 1);     

    if(ioctl( sk, SIOCGIFADDR, &ifreq) < 0) {     
        return -1;     
    }       
    sin = (struct sockaddr_in *)&ifreq.ifr_addr;     

    close(sk);  

    return sin->sin_addr.s_addr;         
}  
int get_local_ip(char *ifname,char *ipaddr)  
{  
    int sk;  
    struct   sockaddr_in *sin;  
    struct   ifreq ifreq;     

    if ((sk=socket(AF_INET, SOCK_STREAM, 0)) == -1)  
    {  
        printf("socket create failse...GetLocalIp!\n");  
        return -1;  
    }  

    memset(&ifreq, 0, sizeof(ifreq));     
    strncpy(ifreq.ifr_name, ifname, sizeof(ifreq.ifr_name) - 1);     

    if(ioctl( sk, SIOCGIFADDR, &ifreq) < 0) {     
        return -1;     
    }       
    sin = (struct sockaddr_in *)&ifreq.ifr_addr;     
    strcpy(ipaddr,inet_ntoa(sin->sin_addr));         

    close(sk);  
      
    return 1;  
}  
int set_local_ip(char *ifname,const char *ipaddr)  
{  
    int sk;  
    struct sockaddr_in sin;  
    struct ifreq ifr_set_ip;  
  
    bzero( &ifr_set_ip,sizeof(ifr_set_ip));  
   
    if( ipaddr == NULL )  
        return -1;  
  
    if((sk = socket( AF_INET, SOCK_STREAM, 0 )) == -1){  
        perror("socket create failse...SetLocalIp!\n");  
        return -1;  
    }  
   
    memset( &sin, 0, sizeof(sin));  
    strncpy(ifr_set_ip.ifr_name, ifname, sizeof(ifr_set_ip.ifr_name)-1);     
      
    sin.sin_family = AF_INET;  
    sin.sin_addr.s_addr = inet_addr(ipaddr);  
    memcpy( &ifr_set_ip.ifr_addr, &sin, sizeof(sin));  
  
    if( ioctl( sk, SIOCSIFADDR, &ifr_set_ip) < 0 )  
    {  
        perror( "Not setup interface\n");  
        return -1;  
    }  
  
    //设置激活标志  
    ifr_set_ip.ifr_flags |= IFF_UP |IFF_RUNNING;  
  
    //get the status of the device  
    if( ioctl( sk, SIOCSIFFLAGS, &ifr_set_ip ) < 0 )  
    {  
         perror("SIOCSIFFLAGS\n");  
         return -1;  
    }  
  
    close( sk );  
    return 0;  
} 

int get_local_netmask(char *ifname,char *netmask_addr)  
{  
    int sk;  
    struct ifreq ifreq;  
    struct sockaddr_in *net_mask;  
          
    sk = socket( AF_INET, SOCK_STREAM, 0 );  
    if( sk == -1)  {  
        perror("create socket failture...GetLocalNetMask\n");  
        return -1;  
    }  
      
    memset(&ifreq, 0, sizeof(ifreq));     
    strncpy(ifreq.ifr_name, ifname, sizeof(ifreq.ifr_name )-1);     
  
    if( (ioctl( sk, SIOCGIFNETMASK, &ifreq ) ) < 0 ) {  
        printf("mac ioctl error\n");  
        return -1;  
    }  
      
    /*
     *net_mask = ( struct sockaddr_in * )&( ifreq.ifr_netmask );  
     *strcpy( netmask_addr, inet_ntoa( net_mask -> sin_addr ) );  
     */
      
    close( sk );  
    return 1;  
}

int set_local_netmask(char *ifname,char *netmask_addr)  
{  
    int sk;  
    struct ifreq ifreq;  
    struct sockaddr_in *sin;  
          
    sk = socket( AF_INET, SOCK_STREAM, 0 );  
    if( sk == -1) {  
        perror("Not create network socket connect\n");  
        return -1;  
    }  
      
    memset(&ifreq, 0, sizeof(ifreq));     
    strncpy(ifreq.ifr_name, ifname, sizeof(ifreq.ifr_name )-1);     

    sin = (struct sockaddr_in *)&ifreq.ifr_addr;  
    sin->sin_family = AF_INET;  
    inet_pton(AF_INET, netmask_addr, &sin->sin_addr);  
  
    if(ioctl(sk, SIOCSIFNETMASK, &ifreq ) < 0) {  
        printf("sk ioctl error\n");  
        return -1;  
    }  

    return 1;
}  

int get_broadcast_addr(char *ifname, char *ipaddr)
{
    int sk;
    struct sockaddr_in  sin;
    struct ifreq ifr;

    sk = socket(AF_INET, SOCK_DGRAM, 0);  
    if (sk == -1)
    {  
        dbg_str(DBG_ERROR,"get_broadcast_addr");  
        return  -1; 
    } 

    memset(&ifr, 0, sizeof(ifr));     
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);

    if (!(ioctl(sk, SIOCGIFBRDADDR, &ifr)))
    {
        memcpy(&sin, &ifr.ifr_addr, sizeof(ifr.ifr_addr));
        strcpy(ipaddr,inet_ntoa(sin.sin_addr));         
    }   

    close(sk);

    return 1;
}

/*
 *int set_broadcast_addr(char *ifname,char *ipaddr)  
 *{  
 *    int sk;  
 *    struct ifreq ifreq;  
 *    struct sockaddr_in *sin;  
 *          
 *    sk = socket( AF_INET, SOCK_STREAM, 0 );  
 *    if( sk == -1) {  
 *        perror("Not create network socket connect\n");  
 *        return -1;  
 *    }  
 *      
 *    memset(&ifreq, 0, sizeof(ifreq));     
 *    strncpy(ifreq.ifr_name, ifname, sizeof(ifreq.ifr_name )-1);     
 *
 *    sin = (struct sockaddr_in *)&ifreq.ifr_addr;  
 *    sin->sin_family = AF_INET;  
 *    inet_pton(AF_INET, ipaddr, &sin->sin_addr);  
 *  
 *    if(ioctl(sk, SIOCSIFBRDADDR, &ifreq ) < 0) {  
 *        printf("sk ioctl error\n");  
 *        return -1;  
 *    }  
 *
 *    return 1;
 *}  
 */


int inet_str2num(int af,char *ip)
{
    struct in_addr in;

    inet_pton(af,ip,&in);
    /*
     *dbg_str(DBG_DETAIL,"ip str to num:%s->%x",ip, in.s_addr);
     */

    return in.s_addr;
}

int inet_num2str(int af,uint32_t ip_num,char *ip_str)
{
    struct in_addr s;

    s.s_addr = ip_num;

    inet_ntop(af, (void *)&s, ip_str, 16);

    /*
     *dbg_str(DBG_DETAIL,"ip num to str:%x->%s",s.s_addr,ip_str);
     */

    return 0;
}

int inet_is_in_same_subnet(char *ip_str,char *net_if_name)
{
    char ip_addr[50];
    char net_mask_addr[50];
    uint32_t ip1, ip2, net_mask;

    get_local_ip(net_if_name,ip_addr);
    ip1 = inet_str2num(AF_INET,ip_addr);
    ip2 = inet_str2num(AF_INET,ip_str);
    /*
     *dbg_str(DBG_DETAIL,"ip=%x",ip2);
     */

    get_local_netmask(net_if_name,net_mask_addr); 
    /*
     *print_ipaddr(net_mask_addr);
     */
    net_mask = inet_str2num(AF_INET,net_mask_addr);
    /*
     *dbg_str(DBG_DETAIL,"net_mask=%x",net_mask);
     */

    if((ip1 & net_mask) == (ip2 & net_mask)) {
        return 1;
    } else {
        return 0;
    }
}

int test_miscellany_net()
{
    unsigned char  mac_addr[6];
    char ip_addr[100];

#if 0
    get_mac((char *)"eth0",mac_addr, sizeof(mac_addr));
    print_mac(mac_addr);

    get_local_ip((char *)"eth0",ip_addr);
    print_ipaddr(ip_addr);

    set_local_ip((char *)"eth0","192.168.20.182");
    get_local_ip((char *)"eth0",ip_addr);
    print_ipaddr(ip_addr);

    dbg_str(DBG_DETAIL,"get local netmask");
    get_local_netmask((char *)"eth0",ip_addr); 
    print_ipaddr(ip_addr);
    dbg_str(DBG_DETAIL,"set local netmask");
    set_local_netmask((char *)"eth0",(char *)"255.255.255.0");  
    get_local_netmask((char *)"eth0",ip_addr); 
    print_ipaddr(ip_addr);

    dbg_str(DBG_DETAIL,"ip and str convert");
    inet_str2num(AF_INET,(char *)"192.168.20.2");
    
    inet_num2str(AF_INET,34908352,ip_addr);
#endif

#if 0
    if(inet_is_in_same_subnet("192.168.2.32","eth0")) {
        /*
         *dbg_str(DBG_DETAIL,"this ip addr add local addr is in the same net");
         */
    } else {
        dbg_str(DBG_DETAIL,"this ip addr add local addr is not in the same net");
    }
#endif

    get_broadcast_addr("eth0", ip_addr);
    print_ipaddr(ip_addr);

    /*
     *char ip_addr2[100];
     *set_broadcast_addr("eth0", "192.168.20.255");
     *print_ipaddr(ip_addr2);
     */

    return 0;
}

