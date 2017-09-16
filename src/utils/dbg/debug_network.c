/*
 * =====================================================================================
 *
 *       Filename:  network.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/24/2015 03:49:59 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
/*  
 * Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 *  
 *  
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
/*
 *#include <stdio.h>  
 *#include <sys/ioctl.h>  
 *#include <sys/socket.h>  
 *#include <sys/types.h>  
 *#include <netdb.h>  
 *#include <net/if.h>  
 *#include <arpa/inet.h>  
 *#include <stdlib.h>
 *#include <string.h>
 *#include <unistd.h>
 *#include "libdbg/debug.h"
 *#include "libcre/sync_lock/sync_lock.h"
 */


//
//const char *get_ipaddr(void)  
//{  
//    int sfd, intr;  
//    struct ifreq buf[16];  
//    struct ifconf ifc;  
//
//    sfd = socket (AF_INET, SOCK_DGRAM, 0);   
//    if (sfd < 0)  
//        return ERRORIP;  
//
//    ifc.ifc_len = sizeof(buf);  
//    ifc.ifc_buf = (caddr_t)buf;  
//    if (ioctl(sfd, SIOCGIFCONF, (char *)&ifc))  
//        return ERRORIP;  
//
//    intr = ifc.ifc_len / sizeof(struct ifreq);  
//    while (intr-- > 0 && ioctl(sfd, SIOCGIFADDR, (char *)&buf[intr]));  
//
//    close(sfd);  
//
//    return inet_ntoa(((struct sockaddr_in*)(&buf[intr].ifr_addr))-> sin_addr);  
//}
//
//
//
//暂未考虑并发情况，后面改进
/*
 *int network_print_sendto(debugger_t *debugger,void *data,size_t len)
 *{
 *    int ret;
 *    int sd = debugger->priv.net.sd;
 *    struct sockaddr_in *raddr = &debugger->priv.net.raddr;
 *
 *    if((ret = sendto(sd,data,len,0,(struct sockaddr *)raddr,sizeof(struct sockaddr_in)))<0){
 *        //PE("send error,errno =%d\n",ret);
 *        //exit(1);
 *    }   
 *    return ret;
 *}
 *
 *
 *void network_print_init(debugger_t *debugger)
 *{
 *#define MAX_NETWORK_INIT_BUF_LEN 20
 *    int sd;
 *    struct sockaddr_in *raddr;
 *    debug_network_prive_t *net_priv = &debugger->priv.net;
 *    dictionary *d = debugger->d;;
 *    char *ip;
 *    uint32_t default_dbg_port = 10276;
 *    char buf[MAX_NETWORK_INIT_BUF_LEN];
 *    char default_dbg_ip[] = "127.0.0.1";
 *
 *    printf("network print init\n");
 *    net_priv->port = iniparser_getint(d, (char *)"network:debug_port",default_dbg_port);
 *    ip = iniparser_getstr(d, (char *)"network:debug_ip");
 *    if(ip == NULL){
 *        memcpy(net_priv->ip_str,default_dbg_ip,strlen(default_dbg_ip));
 *        iniparser_setstr(d, (char *)"network", NULL); 
 *        debug_string_itoa(default_dbg_port,buf,10);
 *        iniparser_setstr(d, (char *)"network:debug_port",buf);
 *        iniparser_setstr(d, (char *)"network:debug_ip", net_priv->ip_str);
 *
 *        FILE *f = fopen(debugger->ini_file_name, "w");
 *        iniparser_dump_ini(d, f);
 *        fclose(f);
 *    }
 *    else 
 *        memcpy(net_priv->ip_str,ip,strlen(ip));
 *
 *    sd = socket(AF_INET,SOCK_DGRAM,0);
 *    if(sd <0){
 *        printf("create socket\n");
 *        exit(1);
 *    }   
 *    int val=1;
 *    if(setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&val,sizeof(val))<0){
 *        printf("setsocgketopt error\n");
 *        exit(1);
 *    }      
 *    //pthread_mutex_init(&debugger->priv.net.send_dgram_lock,NULL);
 *    sync_lock_init(&debugger->priv.net.send_dgram_lock,debugger->lock_type);
 *    debugger->priv.net.sd = sd;
 *
 *    raddr = &debugger->priv.net.raddr;
 *    raddr->sin_family = AF_INET;
 *    raddr->sin_port = htons(net_priv->port);
 *    inet_pton(AF_INET,net_priv->ip_str,&raddr->sin_addr);
 *#undef MAX_NETWORK_INIT_BUF_LEN 
 *}
 *void network_print_destroy(debugger_t *debugger)
 *{
 *    //pthread_mutex_destroy(&debugger->priv.net.send_dgram_lock);
 *    sync_lock_destroy(&debugger->priv.net.send_dgram_lock);
 *}
 *int network_print_print_str_vl(debugger_t *debugger,size_t level,const char *fmt,va_list vl)
 *{
 *    char dest[1024];
 *    size_t ret,offset=0;
 *    uint16_t slen = 1024;
 *    //pthread_mutex_t *lock = &debugger->priv.net.send_dgram_lock;
 *    //pthread_mutex_lock(lock);
 *    sync_lock(&debugger->priv.net.send_dgram_lock,NULL);
 *    memset(dest,'\0',slen);
 *    offset = vsnprintf(dest,slen,fmt,vl);
 *    offset += snprintf(dest + offset,slen,"\n");
 *    ret = network_print_sendto(debugger,dest,offset);
 *    sync_unlock(&debugger->priv.net.send_dgram_lock);
 *    //pthread_mutex_unlock(lock);
 *
 *    return ret;
 *}
 *int network_print_print_str(debugger_t *debugger,size_t level,const char *fmt,...)
 *{
 *    int ret;
 *    va_list ap;
 *
 *    va_start(ap,fmt);
 *    ret = network_print_print_str_vl(debugger,level,fmt,ap);
 *    va_end(ap);
 *
 *    return ret;
 *}
 *void network_print_regester()
 *{
 *    debugger_module_t dm={
 *        .dbg_ops ={
 *            .dbg_string_vl = network_print_print_str_vl,
 *            .dbg_string    = network_print_print_str,
 *            .init          = network_print_init,
 *            .destroy       = network_print_destroy,
 *        }
 *    };
 *    memcpy(&debugger_modules[DEBUGGER_TYPE_NETWORK],&dm,sizeof(debugger_module_t));
 *}
 */
