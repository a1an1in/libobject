/**
 * @file class_deamon.c
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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/miscellany/buffer.h>
#include <libobject/core/class_deamon.h>
#include <libobject/core/class_info.h>
#include <libobject/attrib_priority.h>

class_deamon_t *global_class_deamon;

class_deamon_t * class_deamon_alloc(allocator_t *allocator)
{
    class_deamon_t *class_deamon;

    class_deamon = (class_deamon_t *)allocator_mem_alloc(allocator, sizeof(class_deamon_t));
    if (class_deamon == NULL) {
        dbg_str(OBJ_DETAIL, "allocator_mem_alloc");
        return class_deamon;
    }
    memset(class_deamon, 0, sizeof(class_deamon_t));

    class_deamon->allocator = allocator;

    return class_deamon;
}

int class_deamon_set(class_deamon_t *class_deamon, char *attrib, char *value)
{
    if ((!strcmp(attrib, "map_type")) == 0) {
        class_deamon->map_type = atoi(value);
    } else {
        dbg_str(OBJ_DETAIL, "class_deamon set, not support %s setting", attrib);
    }

    return 0;
}

int class_deamon_init(class_deamon_t *class_deamon)
{
    if (class_deamon->map_type == 0) {
        /*
         *class_deamon->map_type = MAP_TYPE_HASH_MAP;
         */
        class_deamon->map_type = MAP_TYPE_RBTREE_MAP;
    }
    if (class_deamon->map_value_size == 0) {
        class_deamon->map_value_size = sizeof(void *);
    }
    if (class_deamon->map_key_len == 0) {
        class_deamon->map_key_len = 20;
    }

    class_deamon->map = (map_t *)map_alloc(class_deamon->allocator, class_deamon->map_type);
    if (class_deamon->map == NULL) {
        dbg_str(OBJ_ERROR, "map_alloc");
        return -1;
    }

    map_set(class_deamon->map, "key_cmp_func", (void *)string_key_cmp_func);
    map_init(class_deamon->map);

    return 0;
}

int class_deamon_register_class(class_deamon_t *class_deamon, 
                                char *class_name, 
                                void *class_info_addr)
{
    if (class_deamon == NULL) {
        class_deamon = class_deamon_get_global_class_deamon();
    }
    return map_insert(class_deamon->map, class_name, class_info_addr);
}

void * class_deamon_search_class(class_deamon_t *class_deamon, char *class_name)
{
    map_iterator_t it;
    uint8_t *addr;
    int ret;

    ret = map_search(class_deamon->map, class_name, &it);
    if (ret < 0) {
        dbg_str(OBJ_WARNNING, "class_deamon_search_method, not found %s", class_name);
        return NULL;
    }

    addr = (uint8_t *)map_get_pointer(&it);

    return addr;
}

class_deamon_t *class_deamon_get_global_class_deamon()
{
    return global_class_deamon;
}

int class_deamon_destroy(class_deamon_t *class_deamon)
{
    allocator_t *allocator = class_deamon->allocator;

    map_destroy(class_deamon->map);

    allocator_mem_free(allocator, class_deamon);

    return 0;
}

int class_deamon_constructor()
{
    class_deamon_t *class_deamon;
    allocator_t *allocator = allocator_get_default_alloc();

    ATTRIB_PRINT("REGISTRY_CTOR_PRIORITY =%d, run class_deamon\n", 
                 REGISTRY_CTOR_PRIORITY_OBJ_DEAMON);

    class_deamon = class_deamon_alloc(allocator);
    class_deamon_init(class_deamon);

    global_class_deamon = class_deamon;

    return 0;
}
REGISTER_CTOR_FUNC(REGISTRY_CTOR_PRIORITY_OBJ_DEAMON, class_deamon_constructor);

static int class_deamon_destructor()
{
    class_deamon_t *class_deamon = class_deamon_get_global_class_deamon();

    class_deamon_destroy(class_deamon);

    ATTRIB_PRINT("REGISTRY_DTOR_PRIORITY =%d, destruct class deamon alloc count =%d\n", 
                 REGISTRY_DTOR_PRIORITY_OBJ_DEAMON, class_deamon->allocator->alloc_count);

    return 0;
}
REGISTER_DTOR_FUNC(REGISTRY_DTOR_PRIORITY_OBJ_DEAMON, class_deamon_destructor);

