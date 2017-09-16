/*
 * =====================================================================================
 *
 *       Filename:  pa_admin.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  11/04/2015 05:16:03 PM
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
#include <libobject/utils/dbg/debug.h>
#include <libobject/utils/proto_analyzer/protocol_analyzer.h>
#include <libobject/utils/proto_analyzer/protocol_format_set.h>
#include <libobject/utils/proto_analyzer/pa_admin.h>

extern void pfs_set_pdt_drv_proto_format(protocol_format_set_t *pfs_p);

typedef long long uint64_t;
pa_admin_t *pa_admin;

pa_admin_t *pa_admin_create(allocator_t *allocator,
                            uint8_t key_size,uint8_t bucket_size)
{
    pa_admin_t *admin = NULL;
    hash_map_t *hmap = NULL;
    uint32_t value_size;
    pair_t *pair;

    admin = (pa_admin_t *)allocator_mem_alloc(allocator,sizeof(pa_admin_t));
    if(admin == NULL){
        dbg_str(DBG_ERROR,"allocator_mem_alloc");
        goto err_alloc_admin;
    }
    memset(admin,0,sizeof(pa_admin_t));

    admin->allocator = allocator;

    hmap = hash_map_alloc(allocator);
    if(hmap == NULL){
        dbg_str(DBG_ERROR,"hash_map_create");
        goto err_create_hash_map;
    }
    value_size = sizeof(struct protocol_analyzer_s *);
    hash_map_init(hmap, key_size,//uint32_t key_size,
                  value_size ,//uint32_t value_size
                  bucket_size);
    admin->hash_map = hmap;

    pair = create_pair(key_size,value_size);
    if(pair == NULL){
        dbg_str(DBG_ERROR,"hash_map_create");
        goto err_create_pair;
    }
    admin->pair = pair;
    return admin;

err_create_pair:
    hash_map_destroy(admin->hash_map);
err_create_hash_map:
    allocator_mem_free(allocator,admin);
err_alloc_admin:
    return NULL;
}

void pa_admin_destroy_pa(struct hash_map_s *hmap)
{
	hash_map_pos_t pos,next;
    uint8_t *addr_p;
    struct protocol_analyzer_s *pa;
    uint64_t addr = 0;
    uint8_t i;

	for(	hash_map_begin(hmap,&pos),hash_map_pos_next(&pos,&next); 
			!hash_map_pos_equal(&pos,&hmap->end);
			pos = next,hash_map_pos_next(&pos,&next)){
        addr_p = (uint8_t *)hash_map_pos_get_pointer(&pos);

        for(i = 0; i < sizeof(pa); i++){
            addr = (addr << 8 | addr_p[i]);
        }
        pa = (struct protocol_analyzer_s *)addr;
        pa_destroy_protocol_analyzer(pa);
    }
}

void pa_admin_destroy(pa_admin_t *admin)
{
    allocator_t *allocator = admin->allocator;;

    dbg_str(DBG_VIP,"pa_admin_destroy");
    destroy_pair(admin->pair);

    pa_admin_destroy_pa(admin->hash_map);

    hash_map_destroy(admin->hash_map);

    pfs_destroy_protocol_format_set(admin->pfs);
    allocator_mem_free(allocator,admin);

}
/*
 *向 pa_admin注册协议解析器
 * */
void pa_admin_add_protocol_analyzer(pa_admin_t *admin,void *key,struct protocol_analyzer_s *pa)
{
    uint8_t addr_buffer[8];
    uint64_t addr;
    uint8_t i;

    addr = (uint64_t)pa;

    for(i = 0; i < sizeof(pa); i++){
        addr_buffer[i] = (addr >> ((sizeof(pa) - 1 - i) * 8)) & 0xff;
    }

    /*
     *dbg_str(DBG_DETAIL,"pa_admin_add_protocol_analyzer,key=%s,pa addr=%p",key,pa);
     */
    make_pair(admin->pair,key,(void *)addr_buffer);
    hash_map_insert_data(admin->hash_map,admin->pair->data);
}

/*
 *通过key，索引想用的解析器
 * */
struct protocol_analyzer_s *
pa_admin_get_protocol_analyzer(pa_admin_t *admin ,void *key)
{
    hash_map_pos_t pos;
    uint8_t *addr_p;
    struct protocol_analyzer_s *pa;
    uint64_t addr = 0;
    int ret;
    uint8_t i;

    ret = hash_map_search(admin->hash_map, key,&pos);
    if(ret < 0) return NULL;

    if(pos.hlist_node_p == NULL){
        dbg_str(DBG_WARNNING,"not find relevent pa");
        pa = NULL;
    }else{
        addr_p = (uint8_t *)hash_map_pos_get_pointer(&pos);

        for(i = 0; i < sizeof(pa); i++){
            addr = (addr << 8 | addr_p[i]);
        }
        pa = (struct protocol_analyzer_s *)addr;

        pa_reset_vlen_flag(pa);
    }
    return pa;
}

void pa_admin_generate_key(int proto_num,char *key)
{
    sprintf(key,"pa%x",proto_num);
}
void pa_admin_register_protocol_analyzers(pa_admin_t *admin,
                                          protocol_format_set_t *pfs)
{
    struct protocol_analyzer_s *pa;
    allocator_t *allocator = admin->allocator;;
    struct list_head **hl_head_p2 = pfs->list_head_p2;
    uint32_t protocol_num = pfs->proto_max_num;
    uint8_t i = 0;
#define MAX_KEY_LEN 10
    char key[MAX_KEY_LEN];
#undef MAX_KEY_LEN

    admin->pfs = pfs;

    if(!hl_head_p2){
        dbg_str(DBG_ERROR,"hl_head is NULL");
        return;
    }
    if(protocol_num > 100){
        dbg_str(DBG_ERROR,"proto num err");
    }
    /*
     *把协议格式集中注册的协议格式，在pa_admin注册个协议分析器，
     *这样在需要前就提前生成，提高代码性能
     * */
    for(i = 0; i < protocol_num; i++){
        if(hl_head_p2[i] != NULL){
            pa = pa_create_protocol_analyzer(allocator);
            pa_init_protocol_analyzer(pfs->proto_base_addr + i, pfs, pa);
            pa_admin_generate_key(pfs->proto_base_addr + i,key);
            pa_admin_add_protocol_analyzer(admin,key,pa);
        }
    }
}

void test_pa_admin()
{
    protocol_format_set_t *pfs_p;
    uint32_t ret = 0;
    allocator_t *allocator = allocator_get_default_alloc();
    pa_admin_t *pa_admin;
    struct protocol_analyzer_s *pa;

    pfs_p = pfs_create_proto_format_set(allocator);
    init_proto_format_set(0x3000,100,pfs_p);
    pfs_set_pdt_drv_proto_format(pfs_p);

    pa_admin = pa_admin_create(allocator,7, 40);

    pa_admin_register_protocol_analyzers(pa_admin,pfs_p);

    pa = pa_admin_get_protocol_analyzer(pa_admin ,(void *)PA_0X3008);
    if(pa != NULL){
        dbg_str(DBG_DETAIL,"find pa,addr=%p",pa);
    }else{
        dbg_str(DBG_DETAIL,"not find pa");
    }

    pa_admin_destroy(pa_admin);
}
