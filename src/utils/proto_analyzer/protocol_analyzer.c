/*
 * =====================================================================================
 *
 *       Filename:  protocol_analyzer.cpp
 *    Description:  
 *        Version:  1.0
 *        Created:  05/05/2015 19:35:28 
 *       Revision:  none
 *       Compiler:  gcc
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
/*
 *#include <math.h>
 */
#include "libobject/utils/dbg/debug.h"
#include "libobject/utils/proto_analyzer/protocol_analyzer.h"
#include "libobject/utils/data_structure/hash_list.h"

static inline uint32_t 
pa_pow(uint32_t x,uint32_t y)
{

    uint32_t pow_value = 1;
    uint32_t i;
    
    for(i = 0;i < y; i++)
        pow_value *= x;

    return pow_value;
}
uint8_t get_bit_data(uint32_t data,uint8_t bit_pos,uint8_t len)
{
    return (data >> bit_pos) & ((int) pa_pow(2, len) - 1);
}
uint8_t set_bit_data(uint8_t *buf,uint8_t data,uint8_t bit_pos,uint8_t len)
{
    *buf |= ((data & ((int) pa_pow(2, len) - 1)) << bit_pos);
    return 0;
}
int pa_check_protocol_num(struct protocol_analyzer_s *pa)
{
    uint32_t protocol_num = pa->protocol_num;
    uint32_t proto_base_addr = pa->pfs_p->proto_base_addr;
    uint32_t proto_max_num = pa->pfs_p->proto_max_num;
    dbg_str(PA_DETAIL,"proto_base_addr =%x,proto_max_num=%d",proto_base_addr,proto_max_num);
    if( 
        (uint32_t)protocol_num > proto_base_addr + proto_max_num ||
        (uint32_t)protocol_num < proto_base_addr)
    {
        dbg_str(PA_ERROR,"protocol num is err,please check");
        return -1;
    }
    return 0;
}
int pa_find_protocol_format(struct protocol_analyzer_s *pa)
{
    int ret = 0;

    if((ret = pa_check_protocol_num(pa)) < 0){

        return ret;
    };
    //dbg_str(PA_DETAIL,"proto_num=%x,proto_base_addr=%x",pa->protocol_num,pa->pfs_p->proto_base_addr);
    pa->pf_list_head_p = pa->pfs_p->list_head_p2[pa->protocol_num - pa->pfs_p->proto_base_addr];
    if(pa->pf_list_head_p == NULL){
        dbg_str(PA_ERROR,"pa not find_protocol_format");
        ret = -1;
    }
    return ret;
}
int pa_copy_protocol_format(struct protocol_analyzer_s *pa)
{
    int ret = 0;
    if(pa->pf_list_head_p == NULL){
        dbg_str(PA_ERROR,"pa_copy_protocol_format,pf_list_head_p == NULL");
        return -1;
    }

    proto_head_list_t *head_list,*head_list_copy;
    proto_info_list_t *info_list,*info_list_copy;
    struct list_head *hl_head = pa->pf_list_head_p;
    struct list_head *pos,*n;

    if(hl_head == NULL){
        dbg_str(PA_ERROR,"a_copy_protocol_format err,hl_head is NULL");
        return -1;
    }
    head_list = container_of(hl_head,proto_head_list_t,list_head);
    head_list_copy = (proto_head_list_t *)allocator_mem_alloc(pa->allocator,sizeof(proto_head_list_t));
    memcpy(head_list_copy,head_list,sizeof(proto_head_list_t));
    head_list_copy->allocator = pa->allocator;
    head_list_copy->list_count = 0;
    INIT_LIST_HEAD(&head_list_copy->list_head);
    list_for_each_safe(pos, n, hl_head) {
        info_list = container_of(pos,proto_info_list_t,list_head);
        info_list_copy = (proto_info_list_t *)allocator_mem_alloc(pa->allocator,sizeof(proto_info_list_t));
        memcpy(info_list_copy,info_list,sizeof(proto_info_list_t));
        pfs_add_list(&info_list_copy->list_head,&head_list_copy->list_head);

    }
    //dbg_str(PA_DETAIL,"copy_protocol_format original");
    //pfs_print_list_for_each(hl_head);
    dbg_str(PA_DETAIL,"copy_protocol_format copy");
    /*
     *pfs_print_list_for_each(&head_list_copy->list_head);
     */
    pa->pa_list_head_p = &head_list_copy->list_head;
    return ret;
}
void pa_addr_to_buffer(void *addr,uint8_t *buffer)
{
    unsigned long data = (unsigned long)addr;
    int i;

    for(i = 0; i < sizeof(int *); i++){
        buffer[i] = data >> 8 * (sizeof(int *) - 1 - i);
    }
}
void *pa_buffer_to_addr(uint8_t *buffer)
{
    void *ret = NULL;
    unsigned long d = 0, t = 0;
    int i;

    for ( i = 0; i < sizeof(int *); i++){
        t = buffer[i];
        d |= t << 8 * (sizeof(int *) - 1 - i); 
    }

    ret = (void *)d;
    return ret;
}
int pa_create_hash_table(struct protocol_analyzer_s *pa)
{
    hash_map_t *hmap;
    pair_t *pair;
    struct hash_map_node *mnode;
    uint8_t key_size = 15;
    uint8_t value_size = sizeof(proto_info_list_t *);
    uint8_t bucket_size = 10;
    int ret = 0;
    struct list_head *hl_head = pa->pa_list_head_p;
    struct list_head *pos,*n;
    proto_head_list_t *head_list;
    proto_info_list_t *info_list;
    uint8_t addr_buffer[8];
    uint32_t addr;

    //change later,not relase it
    pair = create_pair(key_size,value_size);

    hmap = hash_map_alloc(pa->allocator);
    hash_map_init(hmap,
            key_size,//uint32_t key_size,
            value_size,
            bucket_size);

    if(hl_head == NULL){
        dbg_str(PA_ERROR,"a_copy_protocol_format err,hl_head is NULL");
        return -1;
    }
    head_list = container_of(hl_head,proto_head_list_t,list_head);

    list_for_each_safe(pos, n, hl_head) {
        info_list = container_of(pos,proto_info_list_t,list_head);
        pa_addr_to_buffer(info_list,addr_buffer);
        make_pair(pair,info_list->name,addr_buffer);
        hash_map_insert_data(hmap,pair->data);
    }

    pa->hmap = hmap;

    /*
     *hash_map_for_each(hmap,hash_map_print_mnode);
     *hash_map_destroy(hmap);
     */

    return ret;
}
proto_info_list_t * pa_find_key(const char *key,struct protocol_analyzer_s *pa)
{
    hash_map_t *hmap = pa->hmap;
    hash_map_pos_t map_pos;
    proto_info_list_t *info_list; 
    uint8_t *addr_p;
    uint8_t key_str[hmap->key_size];

    if(strlen(key) < hmap->key_size){
        memset(key_str,0,hmap->key_size);
        memcpy(key_str,key,strlen(key));
    }

    hash_map_search(hmap,(void *)key_str,&map_pos);
    if(map_pos.hlist_node_p == NULL){
        dbg_str(PA_WARNNING,"not found key:%s",key_str);
        return NULL;    
    }
    addr_p = hash_map_pos_get_pointer(&map_pos);
    info_list = pa_buffer_to_addr(addr_p);

    return info_list;
}
struct protocol_analyzer_s *pa_create_protocol_analyzer(allocator_t *allocator)
{
    struct protocol_analyzer_s *ret;

    ret = (struct protocol_analyzer_s *)allocator_mem_alloc(allocator,sizeof(struct protocol_analyzer_s));
    memset(ret,0,sizeof(struct protocol_analyzer_s));
    ret->allocator = allocator;

    return ret;
}
void pa_init_protocol_analyzer(uint32_t proto_no,
        protocol_format_set_t *pfp,
        struct protocol_analyzer_s *pa)
{
    pa->protocol_num = proto_no;
    pa->pfs_p = pfp;
    pa_find_protocol_format(pa);
    pa_copy_protocol_format(pa);
    pa_create_hash_table(pa);
}
void pa_destroy_protocol_analyzer(struct protocol_analyzer_s *pa)
{
    pa->pfs_p = NULL;
    pa->pf_list_head_p = NULL;
    pfs_del_list_for_each(pa->pa_list_head_p);
    pa->pa_list_head_p = NULL;
    hash_map_destroy(pa->hmap);
    allocator_mem_free(pa->allocator,pa);
}
//there may have problem,mem managment
int pa_set_buf(const char *key,uint8_t *data,uint32_t len,struct protocol_analyzer_s *pa)
{
    proto_info_list_t *info_list = NULL; 
    proto_head_list_t *head_list;
    struct list_head *hl_head = pa->pa_list_head_p;

    head_list = container_of(hl_head,proto_head_list_t,list_head);

    info_list = pa_find_key(key,pa);
    if(info_list != NULL){
        if(info_list->buf.data_p == NULL){
            info_list->buf.data_p =(uint8_t *)allocator_mem_alloc(pa->allocator,len);
            info_list->buf.len = len;
            memcpy(info_list->buf.data_p,data,len);
        }else if(len <= info_list->buf.len){
            memcpy(info_list->buf.data_p,data,len);
        }else{
            dbg_str(PA_WARNNING,"prev buffer len is small,release it and new a buffer");
            allocator_mem_free(pa->allocator,info_list->buf.data_p);
            info_list->buf.data_p =(uint8_t *)allocator_mem_alloc(pa->allocator,len);
            info_list->buf.len = len;
            memcpy(info_list->buf.data_p,data,len);
        }
        if(info_list->len != len){
            dbg_str(PA_WARNNING,"proto format len not equal copy data len,name=%s",info_list->name);
        }
    }else{
        dbg_str(PA_ERROR,"not find info list");
    }

    return 0;

}
int pa_set_value(const char *key,uint32_t value,struct protocol_analyzer_s *pa)
{
    proto_info_list_t *info_list = NULL; 

    info_list = pa_find_key(key,pa);
    info_list->data = value;
    return 0;

}
int pa_get_value(const char *key,struct protocol_analyzer_s *pa)
{
    proto_info_list_t *info_list = NULL; 
    info_list = pa_find_key(key,pa);
    if(info_list != NULL)
        return info_list->data;
    else{
        dbg_str(PA_ERROR,"info_list is NULL");
        return -1;
    }
}
int pa_recompute_byte_pos(struct list_head *cur,struct protocol_analyzer_s *pa)
{
    proto_head_list_t *head_list;
    proto_info_list_t *info_list,*info_list_find;
    struct list_head *pos,*list_head_p = pa->pa_list_head_p;
    uint32_t bit_len = 0;
    int ret = 0;
    if(list_head_p == NULL){
        dbg_str(PA_ERROR,"pa_list_head_p is NULL");
        return -1;
    }

    info_list = container_of(cur,proto_info_list_t,list_head);

    dbg_str(PA_DETAIL,"vlenth_index=%s",info_list->vlenth_index);
    info_list_find = pa_find_key(info_list->vlenth_index,pa);
    if(info_list_find == NULL){
        dbg_str(PA_ERROR,"vlenth_flag,but not find related info list,"
                "please check if configure file is right");
        return -1;
    }
    info_list->len = info_list_find->data;
    bit_len = info_list_find->data * info_list->len_unit;

    //recompute byte pos of the info list after vlenth_flag
    head_list = container_of(list_head_p,proto_head_list_t,list_head);
    for(pos = cur->next; pos != list_head_p; pos = pos->next){
        info_list = container_of(pos,proto_info_list_t,list_head);
        info_list ->byte_pos += bit_len / 8;
    }

    return ret;
}
int pa_set_variable_length_flag(struct protocol_analyzer_s *pa)
{
    struct list_head *list_head_p = pa->pa_list_head_p;
    proto_head_list_t *head_list;
    proto_info_list_t *info_list,*info_list_find;;
    struct list_head *pos,*n;
    uint8_t vlenth_flag;
    int ret = 0;

    if(list_head_p == NULL){
        dbg_str(PA_ERROR,"pa_list_head_p is NULL");
        return -1;
    }

    head_list = container_of(list_head_p,proto_head_list_t,list_head);

    list_for_each_safe(pos, n,list_head_p) {
        info_list   = container_of(pos,proto_info_list_t,list_head);
        vlenth_flag = info_list->vlenth_flag;
        if(vlenth_flag == 1){
             info_list_find = pa_find_key(info_list->vlenth_index,pa);
             if(info_list_find != NULL){
                 info_list_find->vlenth_value_flag =1;
             }
        }

    }
    return ret;
}
int pa_set_variable_length_value(char *key,uint32_t value,
        struct protocol_analyzer_s *pa)
{
    proto_info_list_t *info_list;
    int ret = 0;

    info_list = pa_find_key(key,pa);
    if(info_list != NULL) {
        info_list->data = value;
        info_list->vlenth_value_assigned_flag = 1;//已赋值标记
    }else
        ret = -1;

    return ret;
}
void pa_set_protocol_buf(uint8_t *data,uint32_t len,
        uint8_t byte_pos, uint8_t bit_pos,uint8_t *dp)
{
    if(bit_pos != 0){
        dbg_str(PA_ERROR,"not support this set buf while it bit pos is not zero");
        return;
    }
    memcpy(dp + byte_pos,data,len);
}
void pa_set_protocol_byte_data(uint32_t data,
        uint8_t byte_pos, uint8_t bit_pos,
        uint16_t len,uint8_t *dp)
{
    int i;

    //dbg_str(PA_DETAIL,"byte_pos=%d,bit_pos=%d,t_len=%d",byte_pos,bit_pos,t_len);

    for(i = 0; i < len; i++){
        dp[byte_pos + i] = (data >> 8 *(len - i - 1)) & 0xff;
    }
}
void pa_set_protocol_bit_data(uint32_t data,
        uint8_t byte_pos, uint8_t bit_pos,
        uint16_t len,uint8_t *dp)
{
    int i;
    short t_len = len;
    char t_pos_set = bit_pos;
    uint8_t t_data_get;
    uint8_t t_len_get;

    dbg_str(PA_DETAIL,"byte_pos=%d,bit_pos=%d,t_len=%d,data=%x",byte_pos,bit_pos,t_len,data);

    for(i = 0; t_len > 0; i++){
        /* get data len you wana set */
        t_len_get  = (t_len > t_pos_set % 8 + 1)?(t_pos_set % 8 + 1):t_len;
        /*get data you want to set*/
        t_data_get = get_bit_data(data, t_len - t_len_get, t_len_get);
        /* set data */
        set_bit_data(&dp[byte_pos + i],t_data_get,t_pos_set % 8 + 1 - t_len_get,t_len_get);
        dbg_str(PA_DETAIL,"-------t_data_get=%x,t_data_set=%x,t_len_get=%d,t_pos_set=%d",
                t_data_get,dp[byte_pos + i],t_len_get,t_pos_set);
        t_len     -= t_len_get;
        t_pos_set -= t_len_get;
        /* if t_pos_set is less than zero,it must be set 7 */
        if(t_pos_set < 0){
            t_pos_set = 7;
        }
    }
}
void pa_get_protocol_buf(proto_info_list_t *info_list,
        struct protocol_analyzer_s *pa)
{
    if(info_list->bit_pos != 0){
        dbg_str(PA_ERROR,"bit_pos != 0,not support this mod");
    }
    if(info_list->buf.data_p == NULL){
        info_list->buf.data_p = (uint8_t *)allocator_mem_alloc(pa->allocator,info_list->len);
        memcpy(info_list->buf.data_p,pa->protocol_data + info_list->byte_pos,info_list->len);
        info_list->buf.len = info_list->len;
    }else{
        if(info_list->len <= info_list->buf.len){
            memcpy(info_list->buf.data_p,pa->protocol_data + info_list->byte_pos,info_list->len);
        }else{
            dbg_str(PA_WARNNING,"mem has malloc before,but too small,release and realloc");
            allocator_mem_free(pa->allocator,info_list->buf.data_p);
            info_list->buf.data_p = (uint8_t *)allocator_mem_alloc(pa->allocator,info_list->len);
            memcpy(info_list->buf.data_p,pa->protocol_data + info_list->byte_pos,info_list->len);
            info_list->buf.len = info_list->len;
        }
    }

}
void pa_get_protocol_byte_data(proto_info_list_t *info_list,
        struct protocol_analyzer_s *pa)
{
    int i;
    short t_len = info_list->len;
    uint8_t byte_pos = info_list->byte_pos;
    uint8_t *dp = pa->protocol_data;
    uint32_t data = 0;
    //dbg_buf(PA_DETAIL,"data:",dp,pa->protocol_data_len);

    for(i = 0; i < t_len; i++){
        data |= dp[byte_pos + i] << (t_len - i - 1)*8;
    }
    info_list->data = data;
    /*
     *dbg_str(PA_DETAIL,"name %s set data=%x",info_list->name,data);
     */
}
void pa_get_protocol_bit_data(proto_info_list_t *info_list,
        struct protocol_analyzer_s *pa)
{
    int i;
    short total_len = info_list->len;
    uint8_t bit_pos_get = info_list->bit_pos;
    char byte_pos = info_list->byte_pos;
    uint8_t len_get;
    uint8_t *dp = pa->protocol_data;
    uint32_t data = 0,t_data_get;

    for(i = 0; total_len > 0; i++){
        /* get data len you want to get */
        len_get = (total_len > bit_pos_get % 8 + 1)?(bit_pos_get % 8 + 1):total_len;
        /* get data you want to get */
        t_data_get = get_bit_data(dp[byte_pos + i], bit_pos_get % 8 + 1 - len_get, len_get);
        /* restore data to data var*/
        data |= (t_data_get << (total_len - len_get));
        /*
         *dbg_str(PA_DETAIL,"get_data=%x,set_data=%x,len_get =%x,data=%x,bit_pos_get=%x",t_data_get,data,len_get,dp[byte_pos + i],bit_pos_get);
         */
        bit_pos_get -= len_get;
        if(bit_pos_get < 0){
            bit_pos_get = 7;
        }
        total_len -= len_get;
        
    }
    info_list->data = data;
    dbg_str(PA_DETAIL,"name %s set data=%x",info_list->name,data);
}
int pa_generate_protocol_data(struct protocol_analyzer_s *pa)
{
    struct list_head *list_head_p = pa->pa_list_head_p;
    uint8_t *dp = pa->protocol_data;
    uint8_t byte_pos, bit_pos;
    uint16_t len;
    uint8_t len_unit;
    uint32_t data;
    uint8_t vlenth_flag;
    int ret = 0;
    if(list_head_p == NULL){
        dbg_str(PA_ERROR,"pa_list_head_p is NULL");
        return -1;
    }
    proto_head_list_t *head_list;
    proto_info_list_t *info_list;
    struct list_head *pos,*n;

    dbg_str(PA_DETAIL,"pa_generate_protocol_data");

    pa_set_variable_length_flag(pa);

    head_list = container_of(list_head_p,proto_head_list_t,list_head);

    /*
     *pthread_rwlock_rdlock(&head_list->head_lock);
     */
    list_for_each_safe(pos, n,list_head_p) {

        info_list     = container_of(pos,proto_info_list_t,list_head);
        byte_pos      = info_list->byte_pos;
        bit_pos       = info_list->bit_pos;
        len           = info_list->len;
        len_unit      = info_list->len_unit;
        data          = info_list->data;
        vlenth_flag = info_list->vlenth_flag;

        if(     info_list->vlenth_value_flag &&
                !info_list->vlenth_value_assigned_flag)
        {
            dbg_str(PA_WARNNING,"this protocol has variable length,"
                    "please call exclusive func assign value fist");
        }

        //dbg_str(PA_DETAIL,"vlenth_flag =%x",vlenth_flag);
        if(vlenth_flag == 1){
             dbg_str(PA_IMPORTANT,"pa_recompute_byte_pos");
            /*
             *dbg_str(PA_DETAIL,"*************pa_recompute_byte_pos");
             *pfs_print_list_for_each(list_head_p);
             */
             pa_recompute_byte_pos(pos,pa);
            /*
             *dbg_str(PA_DETAIL,"*************pa_recompute_byte_pos");
             *pfs_print_list_for_each(list_head_p);
             */
            len      = info_list->len;
            len_unit = info_list->len_unit;
            info_list->vlenth_flag = 0;
        }

        if(len_unit == 8){//if operate byte data
            if(len <= 4){
                pa_set_protocol_byte_data(data,//uint32_t data,
                        byte_pos,//uint8_t byte_pos, 
                        bit_pos,//uint8_t bit_pos,
                        len,//uint16_t len,
                        dp);//uint8_t *dp)
            }else{
                pa_set_protocol_buf(info_list->buf.data_p,len,
                        byte_pos, bit_pos,dp);
            }
        }else if(len_unit == 1){//if operate bit data
            pa_set_protocol_bit_data(data, byte_pos, bit_pos, len,dp);
        }

    }

    pa->protocol_data_len = byte_pos + len;
    dbg_buf(PA_DETAIL,"buffer:",dp,pa->protocol_data_len);
    return ret;
}
int pa_parse_protocol_data(struct protocol_analyzer_s *pa)
{
    struct list_head *list_head_p;
    int ret = 0;

    if(pa == NULL){
        dbg_str(PA_ERROR,"pa is NULL");
        return -1;
    }
    list_head_p = pa->pa_list_head_p;
    if(list_head_p == NULL){
        dbg_str(PA_ERROR,"pa_list_head_p is NULL");
        return -1;
    }
    if(pa->protocol_data_len == 0){
        dbg_str(PA_ERROR,"protocol_data_len err");
        return -1;
    }

    proto_head_list_t *head_list;
    proto_info_list_t *info_list;
    struct list_head *pos,*n;

    dbg_str(PA_DETAIL,"pa_parse_protocol_data");
    head_list = container_of(list_head_p,proto_head_list_t,list_head);

    list_for_each_safe(pos, n,list_head_p) {
        info_list = container_of(pos,proto_info_list_t,list_head);
        if(info_list->vlenth_flag == 1){
            pa_recompute_byte_pos(pos,pa);
            info_list->vlenth_flag = 0;
        }
        if(info_list->len_unit == 8){
            if(info_list->len <= 4)
                pa_get_protocol_byte_data(info_list,pa);
            else{
                pa_get_protocol_buf(info_list,pa);
            }
        }else if(info_list->len_unit == 1){
            pa_get_protocol_bit_data(info_list, pa);
        }
    }

    return ret;
}
int pa_reset_vlen_flag(struct protocol_analyzer_s *pa)
{
    int ret = 0;
    struct list_head *list_head_p;
    proto_head_list_t *head_list;
    proto_info_list_t *info_list;
    struct list_head *pos,*n;

    if(pa == NULL){
        dbg_str(PA_ERROR,"pa is NULL");
        return -1;
    }
    list_head_p = pa->pa_list_head_p;
    if(list_head_p == NULL){
        dbg_str(PA_ERROR,"pa_list_head_p is NULL");
        return -1;
    }

    head_list = container_of(list_head_p,proto_head_list_t,list_head);
    list_for_each_safe(pos, n,list_head_p) {
        info_list = container_of(pos,proto_info_list_t,list_head);
        if(info_list->vlenth_flag_bak == 1){
            info_list->vlenth_flag = 1;
        }
        info_list->byte_pos = info_list->byte_pos_bak;
        info_list->bit_pos  = info_list->bit_pos_bak;
    }
    return ret;
}
