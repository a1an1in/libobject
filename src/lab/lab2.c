/**
 * @file lab.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2016-10-11
 */
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
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>     /* basic system data types */
#include <sys/socket.h>    /* basic socket definitions */
#include <netinet/in.h>    /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>     /* inet(3) functions */
#include <fcntl.h>         /* nonblocking */
#include <sys/resource.h>  /*setrlimit */
#include <signal.h>
#include <sys/un.h>
#include <libobject/core/utils/dbg/debug.h>


#define CHANNEL_USEABLE_MAX_COUNT 30
uint8_t channel_resources[CHANNEL_USEABLE_MAX_COUNT] = {
    1,6,11,
    36,40,44,48,52,56,60,64,
    100,104,108,112,116,120,124,128,132,136,140,
    149,153,157,161,165};

struct channel_config{
    uint8_t channel;
    uint8_t usable_flag;
}channel_cfg[CHANNEL_USEABLE_MAX_COUNT];
#undef CHANNEL_USEABLE_MAX_COUNT

int get_channel_resource_index(uint8_t chan)
{
    int i = 0;
    
    if (chan == 0 || chan > 165)
        return -1;

    for (i = 0; i < sizeof(channel_resources); i++) {
        if (channel_resources[i] == chan) {
            return i;
        }
    }

    return -1;
}

int set_usable_flag(uint8_t chan) 
{
    int index = 0;

    index = get_channel_resource_index(chan);
    if (index >= 0) {
        channel_cfg[index].channel     = chan;
        channel_cfg[index].usable_flag = 1;
        dbg_str(DBG_DETAIL, "set flag, channel=%d", chan);
    }

    return 0;
}

int init_channel_cfg()
{
    int i;

    for (i = 0; i < sizeof(channel_resources); i++) {
        channel_cfg[i].channel = channel_resources[i];
        channel_cfg[i].usable_flag = 0;
    }

    return -1;
}

int set_exclusive_channel(uint8_t *excluded_chans)
{
    int i, j = 0;

    for (i = 0; i < sizeof(channel_resources); i++) {
        if (channel_cfg[i].usable_flag != 1) {
            excluded_chans[j++] = channel_cfg[i].channel;
        }
    }

    for (i = 0; i < sizeof(channel_resources); i++) {
        if (channel_cfg[i].usable_flag == 1) {
            dbg_str(DBG_DETAIL, "channel=%d is usable", channel_cfg[i].channel);
        }
    }

    return j;
}

int get_exclude_channel(uint8_t *usable_chans, uint8_t *excluded_chans)
{
    uint8_t *chan = usable_chans;
    int i, ret;

    init_channel_cfg();

    while (*chan != 0) {
        set_usable_flag(*chan++); 
    }

    ret = set_exclusive_channel(excluded_chans);

    for (i = 0; i < ret; i++) {
        if (excluded_chans[i] != 0)
            dbg_str(DBG_DETAIL, "channel=%d is excluded", excluded_chans[i]);
    }
    
}

int lab2()
{
    int i;
    int channel;
    uint8_t usable_chans[256] = {36,140,149};
    uint8_t excluded_channels[256] = {0};

    dbg_str(DBG_DETAIL,"lab2 test");

    get_exclude_channel(usable_chans, excluded_channels);

}
