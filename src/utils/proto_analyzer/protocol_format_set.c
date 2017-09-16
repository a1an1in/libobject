/*
 * =====================================================================================
 *
 *       Filename:  protocol_format_set.cpp
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  05/05/2015 19:35:28 
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan 
 *        Company:  vigor
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
#include <ctype.h>
#include "libobject/utils/dbg/debug.h"
#include "libobject/utils/alloc/allocator.h"
#include "libobject/utils/proto_analyzer/protocol_format_set.h"
/*
 *#include "libproto_analyzer/pdt_proto_format.h"
 */


int hexstr2num(const char* const_str)
{
    uint32_t ret=0;
    uint32_t t;
    char * str = (char *)const_str;
    int len = strlen(str);

    if(strncmp(str,"0x",2) && strncmp(str,"0X",2)){
        printf("not hex data err\n");
        return -1;
    };
    if(len > 10){
        printf("str is too long\n");
        return -1;
    }
    for (str += 2; *str; str++)
    {
        if (*str>='A' && *str <='F')
            t = *str-55;//a-f之间的ascii与对应数值相差55如'A'为65,65-55即为A
        else
            t = *str-48;
        ret <<= 4;
        ret |= t;
    }
    return ret;
} 
int str2num(const char *const_str)
{
    char * str = (char *)const_str;
    int ret;

    if(strncmp(str,"0x",2) && strncmp(str,"0X",2)){
        dbg_str(PA_DETAIL,"str is dex data,convert func is atoi");
        ret = atoi(const_str);
    }else{
        dbg_str(PA_DETAIL,"str is hex data,convert func is hexstr2num");
        ret = hexstr2num(const_str);
    }
    return ret;
}
void free_proto_info_list_mem(void *mem_used)
{
    if(mem_used != NULL){
        free(mem_used);
        mem_used = NULL;
    }   
}
void init_proto_info_list(proto_info_list_t *info_list)
{
    info_list->free_mem = free_proto_info_list_mem;
    info_list->mem_used = NULL;
}
void print_info_list(proto_info_list_t *info_list)
{
    if(info_list == NULL)
        return;
    if(info_list->len <= 4)
        printf("|%15s|%10d|%10d|%10d|%10d|%10d|%10d|%10d|%10x|%10s|\n",
                info_list->name,
                info_list->byte_pos,
                info_list->bit_pos,
                info_list->len,
                info_list->len_unit,
                info_list->vlenth_flag,
                info_list->vlenth_value_flag,
                info_list->vlenth_value_assigned_flag,
                info_list->data,
                info_list->vlenth_index);
    else{
        printf("|%15s|%10d|%10d|%10d|%10d|%10d|%10d|%10d|%10s|%10s|\n",
                info_list->name,
                info_list->byte_pos,
                info_list->bit_pos,
                info_list->len,
                info_list->len_unit,
                info_list->vlenth_flag,
                info_list->vlenth_value_flag,
                info_list->vlenth_value_assigned_flag,
                "N/A",
                info_list->vlenth_index);
        /*
         *if(info_list->buf.data_p != NULL)
         *    dbg_buf(PA_DETAIL,"buf:",info_list->buf.data_p,info_list->len);
         */
    }
}
struct list_head *pfs_create_head_list(allocator_t *allocator)
{
    proto_head_list_t *head_list;

    head_list = (proto_head_list_t *)allocator_mem_alloc(allocator,sizeof(proto_head_list_t));
    if(head_list == NULL){
        dbg_str(PA_ERROR,"create head_list err");
        return NULL;
    }
    /*
     *pthread_rwlock_init(&head_list->head_lock,NULL);
     */
    head_list->allocator = allocator;
    dbg_str(PA_DETAIL,"allocator=%p",allocator);

    return &head_list->list_head;
}
void pfs_init_head_list(struct list_head *hl_head)
{
    proto_head_list_t *head_list;

    head_list = container_of(hl_head,proto_head_list_t,list_head);
    /*
     *pthread_rwlock_init(&head_list->head_lock,NULL);
     */
    head_list->list_count = 0;
    INIT_LIST_HEAD(&head_list->list_head);
}
//释放表头
void pfs_release_head_list(struct list_head *hl_head)
{
    proto_head_list_t *head_list;

    head_list = container_of(hl_head,proto_head_list_t,list_head);
    dbg_str(PA_DETAIL,"release_head_list");
    /*
     *pthread_rwlock_destroy(&head_list->head_lock);
     */
    allocator_mem_free(head_list->allocator,head_list);
}
void pfs_add_list(struct list_head *new_head,struct list_head *hl_head)
{
    proto_head_list_t *head_list;

    head_list = container_of(hl_head,proto_head_list_t,list_head);

    /*
     *pthread_rwlock_wrlock(&head_list->head_lock);
     */
    list_add_tail(new_head,hl_head);
    head_list->list_count++;
    /*
     *pthread_rwlock_unlock(&head_list->head_lock);
     */

}
void pfs_del_list(struct list_head *del_head,struct list_head *hl_head)
{
    proto_head_list_t *head_list;
    proto_info_list_t *info_list;

    head_list = container_of(hl_head,proto_head_list_t,list_head);

    /*
     *pthread_rwlock_wrlock(&head_list->head_lock);
     */
    info_list = container_of(del_head,proto_info_list_t,list_head);

    list_del(del_head);
    head_list->list_count--;
    //there may have bug
    /*
     *info_list->free_mem(info_list->mem_used);
     */
    if(info_list->buf.len > 0)
        allocator_mem_free(head_list->allocator,info_list->buf.data_p);
    allocator_mem_free(head_list->allocator,info_list);
    /*
     *pthread_rwlock_unlock(&head_list->head_lock);
     */

}
void pfs_print_list_for_each(struct list_head *hl_head)
{
    proto_head_list_t *head_list;
    proto_info_list_t *info_list;
    struct list_head *pos,*n;

    head_list = container_of(hl_head,proto_head_list_t,list_head);
    /*
     *pthread_rwlock_rdlock(&head_list->head_lock);
     */
    char c1  [] = "name";
    char c2  [] = "byte_pos";
    char c3  [] = "bit_pos";
    char c4  [] = "len";
    char c5  [] = "len_unit";
    char c6  [] = "vl_flag";
    char c7  [] = "vl_vflag";
    char c8  [] = "vl_vaflag";
    char c9  [] = "value";
    char c10 [] = "vl_index";

    printf("|%15s|%10s|%10s|%10s|%10s|%10s|%10s|%10s|%10s|%10s|\n",
            c1 , c2 , c3 , c4 , c5 , c6 , c7 , c8 , c9, c10);
    list_for_each_safe(pos, n, hl_head) {
        info_list = container_of(pos,proto_info_list_t,list_head);
        print_info_list(info_list);
    }
    /*
     *list_for_each_safe(pos, n, hl_head) {
     *    info_list = container_of(pos,proto_info_list_t,list_head);
     *    print_info_list_buffer(info_list);
     *}
     */
    /*
     *pthread_rwlock_unlock(&head_list->head_lock);
     */
}
void pfs_del_list_for_each(struct list_head *hl_head)
{
    struct list_head *pos,*n;
    int i=0;

    list_for_each_safe(pos, n, hl_head) {
        i++;
        dbg_str(PA_DETAIL,"del %d list",i);
        pfs_del_list(pos,hl_head);
    }
    pfs_release_head_list(hl_head);
}

void pfs_print_proto_link_lists(protocol_format_set_t *pfp)
{
    int i = 0;
    struct list_head **hl_head = pfp->list_head_p2;
    if(!hl_head){
        dbg_str(PA_ERROR,"hl_head is NULL");
        return;
    }
    for(i = 0; i < 100; i++){
        if(hl_head[i] != NULL){
            pfs_print_list_for_each(hl_head[i]);
        }
    }
}
void pfs_del_proto_link_list(protocol_format_set_t *pfp,uint32_t llist_num)
{
    struct list_head **hl_head = pfp->list_head_p2;

    if(!hl_head){
        dbg_str(PA_ERROR,"hl_head is NULL");
        return;
    }
    dbg_str(PA_IMPORTANT,"delete link list %d",llist_num);
    pfs_del_list_for_each(hl_head[llist_num]);
}
void set_proto_info_attribs(char *name,void *attr_value,proto_info_list_t *info_list)
{

    if(!strcmp(name,"name")){
        strcpy(info_list->name,(char *)attr_value);
        /*
         *dbg_buf(PA_DETAIL,"name:",(char *)attr_value,sizeof(attr_value));
         */
    }else if(!strcmp(name,"byte_pos")){
        info_list->byte_pos     = atoi((char *)(attr_value));
        info_list->byte_pos_bak = info_list->byte_pos;
        /*
         *dbg_str(PA_DETAIL,"byte_pos=%d",info_list->byte_pos);
         */
    }else if(!strcmp(name,"bit_pos")){
        info_list->bit_pos     = atoi((char *)(attr_value));
        info_list->bit_pos_bak = info_list->bit_pos;
    }else if(!strcmp(name,"len")){
        info_list->len = atoi((char *)(attr_value));
        /*
         *dbg_str(PA_DETAIL,"len=%d",info_list->len);
         */
        //dbg_str(PA_DETAIL,"len =%d",info_list->len);
        
    }else if(!strcmp(name,"len_unit")){
        info_list->len_unit = atoi((char *)(attr_value));
    }else if(!strcmp(name,"vlenth_index")){
        if(attr_value == NULL){
            return;
        }
        strcpy(info_list->vlenth_index,(char *)attr_value);
        /*
         *dbg_buf(PA_DETAIL,"vlenth_index:",(char *)attr_value,sizeof(attr_value));
         */
        if(strlen(info_list->vlenth_index) > 0){
            info_list->vlenth_flag = 1;
            info_list->vlenth_flag_bak = 1;
        }else{
            strcpy(info_list->vlenth_index,"N/A");
        }
    }else if(!strcmp(name,"value")){
        info_list->bit_len = info_list->len * info_list->len_unit;
        if(info_list->bit_len / 8 <= 4){
            info_list->data = atoi((char *)(attr_value));
            //dbg_str(PA_DETAIL,"data =%x",info_list->data);
        }else {
            info_list->buf.data_p = NULL;
            dbg_str(PA_DETAIL,"value is buf,need malloc mem,this data hasnot stored");
        }
    }else{
    }
}
int pfs_add_proto_link_list(uint32_t llist_num,
        struct list_head *new_proto_llist,
        protocol_format_set_t *pfs_ptr)
{
#define MAX_PROTOCOL_NUM 200
    struct list_head **list_head = pfs_ptr->list_head_p2;
    if(llist_num > MAX_PROTOCOL_NUM){
        dbg_str(PA_ERROR,"max_protocol_num greater than setting");
        return -1;
    }
    if(new_proto_llist == NULL){
        dbg_str(PA_ERROR,"new_proto_llist is NULL");
        return -1;
    }
    list_head[llist_num] = new_proto_llist;
    pfs_ptr->proto_total_num++;
    return 0;
#undef MAX_PROTOCOL_NUM 
}

proto_info_list_t * 
pfs_set_proto_info(char *name,char *name_value,
                   char *byte_pos,char *byte_pos_value,
                   char *bit_pos,char *bit_pos_value,
                   char *len,char *len_value,
                   char *len_unit,char *len_unit_value,
                   char *vlenth_index,char *vlenth_index_value,
                   struct list_head *hl_head)
{
    proto_head_list_t *head_list;
    proto_info_list_t *info_list;

    head_list = container_of(hl_head,proto_head_list_t,list_head);
    info_list = (proto_info_list_t *)allocator_mem_alloc(head_list->allocator,sizeof(proto_info_list_t));
    if(info_list == NULL){
        return NULL;
    }
    memset(info_list,0,sizeof(proto_info_list_t));
    set_proto_info_attribs(name, name_value,info_list);
    set_proto_info_attribs(byte_pos, byte_pos_value,info_list);
    set_proto_info_attribs(bit_pos, bit_pos_value,info_list);
    set_proto_info_attribs(len, len_value,info_list);
    set_proto_info_attribs(len_unit, len_unit_value,info_list);
    set_proto_info_attribs(vlenth_index, vlenth_index_value,info_list);

    pfs_add_list(&info_list->list_head,hl_head);
    return info_list;
}


protocol_format_set_t *
pfs_create_proto_format_set(allocator_t *allocator)
{
    protocol_format_set_t *ret;

    ret = (protocol_format_set_t *)allocator_mem_alloc(allocator,sizeof(protocol_format_set_t));
    if(ret <= 0){
        dbg_str(PA_ERROR,"allocator_mem_alloc err");
    }
    memset(ret,0,sizeof(protocol_format_set_t));
    ret->allocator = allocator;

    return ret;
}
void pfs_destroy_protocol_format_set(protocol_format_set_t *pfp)
{
    uint8_t i = 0;
    struct list_head **hl_head_p2 = pfp->list_head_p2;
    uint32_t protocol_num = pfp->proto_max_num;

    dbg_str(PA_DETAIL,"protocol num=%d",protocol_num);
    if(!hl_head_p2){
        dbg_str(PA_ERROR,"hl_head is NULL");
        return;
    }
    if(protocol_num > 100){
        dbg_str(PA_ERROR,"proto num err");
    }
    for(i = 0; i < protocol_num; i++){
        if(hl_head_p2[i] != NULL){
            pfs_del_proto_link_list(pfp,i);
            hl_head_p2[i] = NULL;
        }
    }
    dbg_str(PA_DETAIL,"allocator=%p free addr=%p",pfp->allocator,hl_head_p2);
    allocator_mem_free(pfp->allocator,hl_head_p2);
    allocator_mem_free(pfp->allocator,pfp);

}
void init_proto_format_set(int proto_base_addr,
                           int max_proto_num,
                           protocol_format_set_t *pfs_p)
{
    pfs_p->proto_base_addr = proto_base_addr;

    pfs_p->list_head_p2 = (struct list_head **)allocator_mem_alloc(pfs_p->allocator,
            sizeof(struct list_head *)*max_proto_num);
    memset(pfs_p->list_head_p2,0,sizeof(struct list_head *)*max_proto_num);
    dbg_str(PA_DETAIL,"protocol num=%d",max_proto_num);
    dbg_str(PA_DETAIL,"allocator=%p free addr=%p",pfs_p->allocator,pfs_p->list_head_p2);
    //not realse
    pfs_p->proto_total_num = 0;
    pfs_p->proto_max_num = max_proto_num;
}
/*
 *int main()
 *{
 *    protocol_format_set_t pfp;
 *    init_proto_format_parser(0x3000,100,&pfp);
 *    pfs_set_proto_format_3008(&pfp);
 *    pfs_print_proto_link_lists(&pfp);
 *    pfs_del_proto_link_lists(&pfp);
 *    dbg_str(PA_DETAIL,"run at here");
 *}
 *
 */
