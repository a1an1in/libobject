/*
 * =====================================================================================
 *
 *       Filename:  test_ctr_alloc.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/15/2015 11:18:21 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
/*  
 * Copyright (c) 2015-2010 alan lin <a1an1in@sina.com>
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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <libobject/core/utils/alloc/allocator.h>
#include <libobject/core/utils/dbg/debug.h>
#include "libobject/user_mode.h"

#if 0
void test_ctr_alloc()
{
    allocator_t *allocator;
    void *p ,*p2,*p3;
    uint32_t size = 8;

    /*
     *alloc_p->slab_max_num = SLAB_ARRAY_MAX_NUM;
     *alloc_p->data_min_size = 8;
     *alloc_p->mempool_capacity = MEM_POOL_MAX_SIZE;
     */
    uint8_t lock_type;
#ifdef UNIX_LIKE_USER_MODE
    lock_type = PTHREAD_MUTEX_LOCK;
#endif
#ifdef WINDOWS_USER_MODE
    lock_type = WINDOWS_MUTEX_LOCK;
#endif
    allocator = allocator_create(ALLOCATOR_TYPE_CTR_MALLOC,0);
    allocator_ctr_init(allocator, 0, 0, 1024);

    printf("\n");
    dbg_str(ALLOC_IMPORTANT,"ctr alloc test begin");
    /*
     *allocator_ctr_init(allocator,0,0,0);
     */

    p = allocator_mem_alloc(allocator,7);
    /*
     *allocator_mem_free(allocator,p);
     */
    p2 = allocator_mem_alloc(allocator,8);
    p3 = allocator_mem_alloc(allocator,200);
    dbg_str(ALLOC_DETAIL,"alloc addr:%p",p3);

    dbg_str(ALLOC_IMPORTANT,"inquire alloc info");
    allocator_mem_info(allocator);

    allocator_mem_free(allocator,p);
    allocator_mem_free(allocator,p2);
    allocator_mem_free(allocator,p3);

    dbg_str(ALLOC_DETAIL,"batch alloc");
    int i;
    for(size = 8,i = 0; i< 20; i++,size += 8){
        p = allocator_mem_alloc(allocator,size);
    }
    dbg_str(ALLOC_WARNNING,"inquire alloc info");
    allocator_mem_info(allocator);

    allocator_destroy(allocator);
    dbg_str(ALLOC_DETAIL,"test ctr alloc end");
}

#elif 1

void test_ctr_alloc()
{
    allocator_t *allocator;
    void *p1 ,*p2,*p3, *p4, *p5;
    void *array[20], *array2[20], *array3[20];
    uint32_t size = 8;
    int cnt = 0;

#if 0
    allocator = allocator_create(ALLOCATOR_TYPE_CTR_MALLOC,1);
    allocator_ctr_init(allocator, 0, 0, 1024);
#else
    allocator = allocator_get_default_alloc();
#endif

    dbg_str(ALLOC_SUC,"ctr alloc test begin");

    p1 = allocator_mem_alloc(allocator,7);
    p2 = allocator_mem_alloc(allocator,8);
    p3 = allocator_mem_alloc(allocator,20);
    allocator_mem_free(allocator, p1);
    allocator_mem_free(allocator, p2);
    allocator_mem_free(allocator, p3);

    while (cnt++ < 10) {
        dbg_str(ALLOC_DETAIL,"batch alloc");
        int i;
        for(size = 8,i = 0; i< 12; i++,size *= 2){
            array[i] = allocator_mem_alloc(allocator,size);
            dbg_str(DBG_DETAIL,"allocator_mem_alloc, size %d, addr:%p", size, array[i]);
        }
        /*
         *for(size = 8,i = 0; i< 12; i++,size *= 2){
         *    array2[i] = allocator_mem_alloc(allocator,size);
         *}
         *for(size = 8,i = 0; i< 12; i++,size *= 2){
         *    array3[i] = allocator_mem_alloc(allocator,size);
         *}
         */

        dbg_str(ALLOC_DETAIL,"batch free");
        for(i = 0; i< 11; i++){
            allocator_mem_free(allocator, array[i]);
            dbg_str(DBG_DETAIL,"allocator_mem_free, addr=%p", array[i]);
        }
        /*
         *for(i = 0; i< 12; i++){
         *    allocator_mem_free(allocator, array2[i]);
         *}
         *for(i = 0; i< 11; i++){
         *    allocator_mem_free(allocator, array3[i]);
         *}
         */
        sleep(1);
    }

    /*
     *dbg_str(ALLOC_VIP,"alloc huge alloc");
     *p4 = allocator_mem_alloc(allocator,1024 * 2);
     *p5 = allocator_mem_alloc(allocator,1024 * 3);
     */
    /*
     *allocator_mem_free(allocator, p4);
     */
    /*
     *allocator_mem_info(allocator);
     */

    /*
     *allocator_destroy(allocator);
     */
    dbg_str(ALLOC_DETAIL,"test ctr alloc end");
}

#else

void* test_thread1_func_cb(void *arg)                                                                           
{                                                                                         
    allocator_t *allocator;
    void *p1 ,*p2,*p3, *p4;
    uint32_t size = 8;

    allocator = allocator_get_default_alloc();

    while(1){
        /*
         *dbg_str(ALLOC_SUC,"test_thread1_func_cb begin");
         */

        p1 = allocator_mem_alloc(allocator,7);
        p2 = allocator_mem_alloc(allocator,8);
        p3 = allocator_mem_alloc(allocator,20);
        allocator_mem_free(allocator, p1);
        allocator_mem_free(allocator, p2);
        allocator_mem_free(allocator, p3);

        /*
         *dbg_str(ALLOC_SUC,"test_thread1_func_cb end");
         */
    }
}                                                                

void* test_thread2_func_cb(void *arg)                                                                           
{                                                                                         
    allocator_t *allocator;
    void *p1 ,*p2,*p3, *p4;
    uint32_t size = 8;

    allocator = allocator_get_default_alloc();

    while(1){
        /*
         *dbg_str(ALLOC_SUC,"test_thread2_func_cb begin");
         */

        p1 = allocator_mem_alloc(allocator,7);
        allocator_mem_free(allocator, p1);
        p2 = allocator_mem_alloc(allocator,8);
        allocator_mem_free(allocator, p2);

        /*
         *dbg_str(ALLOC_SUC,"test_thread2_func_cb end");
         */
    }
}                                                                

void* test_thread3_func_cb(void *arg)                                                                           
{                                                                                         
    allocator_t *allocator;
    void *p1 ,*p2,*p3, *p4;
    uint32_t size = 8;

    allocator = allocator_get_default_alloc();

    while(1){
        /*
         *dbg_str(ALLOC_SUC,"test_thread3_func_cb begin");
         */

        p1 = allocator_mem_alloc(allocator,7);
        allocator_mem_free(allocator, p1);

        /*
         *dbg_str(ALLOC_SUC,"test_thread3_func_cb end");
         */
    }
}                                                                

void test_ctr_alloc()
{
    allocator_t *allocator;
    pthread_t tid1, tid2, tid3;                                     
    int ret;                                                                     

    allocator = allocator_get_default_alloc();

    ret = pthread_create(&tid1,NULL,test_thread1_func_cb,NULL);                                     
    if(ret < 0){                                                                 
        dbg_str(DBG_ERROR,"pthread_create");                                     
        return ;                                     
    }                                                  

    ret = pthread_create(&tid2,NULL,test_thread2_func_cb,NULL);                                     
    if(ret < 0){                                                                 
        dbg_str(DBG_ERROR,"pthread_create");                                     
        return ;                                     
    }                                                  

    ret = pthread_create(&tid3,NULL,test_thread3_func_cb,NULL);                                     
    if(ret < 0){                                                                 
        dbg_str(DBG_ERROR,"pthread_create");                                     
        return ;                                     
    }                                                  
    /*
     *pause();
     */
    sleep(5);
}

#endif
