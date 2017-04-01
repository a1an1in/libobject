/**
 * @file object_deamon.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 1
 * @date 2016-11-21
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
#include <libdbg/debug.h>
#include <libobject/object_deamon.h>
#include <miscellany/buffer.h>
#include <attrib_priority.h>

object_deamon_t *global_object_deamon;

object_deamon_t * object_deamon_alloc(allocator_t *allocator)
{
    object_deamon_t *object_deamon;

    object_deamon = (object_deamon_t *)allocator_mem_alloc(allocator,sizeof(object_deamon_t));
    if (object_deamon == NULL) {
        dbg_str(OBJ_DETAIL,"allocator_mem_alloc");
        return object_deamon;
    }
    memset(object_deamon,0, sizeof(object_deamon_t));

    object_deamon->allocator = allocator;

    return object_deamon;
}

int object_deamon_set(object_deamon_t *object_deamon, char *attrib, char *value)
{
    if (!strcmp(attrib, "map_type") == 0) {
        object_deamon->map_type = atoi(value);
    } else {
        dbg_str(OBJ_DETAIL,"object_deamon set, not support %s setting",attrib);
    }

    return 0;
}

int object_deamon_init(object_deamon_t *object_deamon)
{
    if (object_deamon->map_type == 0) {
        object_deamon->map_type = MAP_TYPE_HASH_MAP;
    }
    if (object_deamon->map_value_size == 0) {
        object_deamon->map_value_size = sizeof(void *);
    }
    if (object_deamon->map_key_len == 0) {
        object_deamon->map_key_len = 20;
    }

    object_deamon->map = (map_t *)map_alloc(object_deamon->allocator,object_deamon->map_type);
    if (object_deamon->map == NULL) {
        dbg_str(OBJ_ERROR,"map_alloc");
        return -1;
    }

    map_init(object_deamon->map, object_deamon->map_key_len, object_deamon->map_value_size);

    return 0;
}

int object_deamon_register_class(object_deamon_t *object_deamon,
                                 char *class_name,
                                 void *class_info_addr)
{
    uint8_t addr_buf[6];

    addr_to_buffer(class_info_addr,addr_buf);
    return map_insert(object_deamon->map,class_name, addr_buf);
}

void * object_deamon_search_class(object_deamon_t *object_deamon, char *class_name)
{
    map_iterator_t it;
    uint8_t *addr;
    int ret;

    ret = map_search(object_deamon->map, class_name, &it);
    if (ret < 0) {
        dbg_str(OBJ_WARNNING,"object_deamon_search_method, not found %s",class_name);
        return NULL;
    }

    addr = (uint8_t *)map_get_pointer(&it);

    return buffer_to_addr(addr);
}

object_deamon_t *object_deamon_get_global_object_deamon()
{
    return global_object_deamon;
}

int object_deamon_destroy(object_deamon_t *object_deamon)
{
    allocator_t *allocator = object_deamon->allocator;

    map_destroy(object_deamon->map);

    allocator_mem_free(allocator,object_deamon);

    return 0;
}

__attribute__((constructor(ATTRIB_PRIORITY_OBJ_DEAMON))) void
object_deamon_constructor()
{
    object_deamon_t *object_deamon;
    allocator_t *allocator = allocator_get_default_alloc();

    ATTRIB_PRINT("constructor ATTRIB_PRIORITY_OBJ_DEAMON =%d, run object_deamon\n",
                 ATTRIB_PRIORITY_OBJ_DEAMON);

    object_deamon = object_deamon_alloc(allocator);
    object_deamon_init(object_deamon);

    global_object_deamon = object_deamon;
}

__attribute__((destructor(ATTRIB_PRIORITY_OBJ_DEAMON))) static void
object_deamon_destructor()
{
    object_deamon_t *object_deamon = object_deamon_get_global_object_deamon();

    object_deamon_destroy(object_deamon);

    ATTRIB_PRINT("destructor ATTRIB_PRIORITY_OBJ_DEAMON =%d, alloc count =%d\n",
                 ATTRIB_PRIORITY_OBJ_DEAMON, object_deamon->allocator->alloc_count);
}

