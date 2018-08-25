/*
 * =====================================================================================
 *
 *       Filename:  debug_network.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/24/2015 10:49:25 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __DEBUG_NETWORK_H__
#define __DEBUG_NETWORK_H__

#include <sys/socket.h>
#include<netinet/in.h>
#include <libobject/core/utils/thread/sync_lock.h>

typedef struct debug_network_prive{
#define MAX_IP_LEN 20
    int sd;
    char ip_str[MAX_IP_LEN];
    unsigned int port;
    //pthread_mutex_t send_dgram_lock;
    sync_lock_t send_dgram_lock;
    struct sockaddr_in raddr;
#undef MAX_IP_LEN
}debug_network_prive_t;
int network_print_regester();

#endif
