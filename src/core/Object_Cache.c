/**
 * @file Object_Cache.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-04-27
 */

/* 
 *  Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *  notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *  notice, this list of conditions and the following disclaimer in the
 *  documentation and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *  derived from this software without specific prior written permission.
 *  
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <libobject/core/Object_Cache.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/String.h>

static int __construct(Object_Cache *cache,char *init_str)
{
    allocator_t *allocator = ((Obj *)cache)->allocator;
    Map *map = NULL;
    int ret = 0;

    map = object_new(allocator, "RBTree_Map", NULL);
    if (map == NULL) {
        dbg_str(DBG_ERROR, "object new object cache error");
        return -1;
    }
    cache->class_map = map;
    map->set_cmp_func(map, string_key_cmp_func);

    cache->object_list = object_new(allocator, "Linked_List", NULL);
    if (cache->object_list == NULL) {
        ret = -1;
        goto error_new_object_list;
    }

    goto end;

error_new_object_list:
    object_destroy(cache->class_map);
end:

    return ret;
}

void __free_object_in_object_list(void *object)
{
    Obj *o = (Obj *)object;

    o->cache = NULL;
    object_destroy(object);
}

static int __deconstruct(Object_Cache *cache)
{
    List *object_list = cache->object_list;

    object_destroy(cache->class_map);
    object_list->for_each(object_list, __free_object_in_object_list);
    object_destroy(object_list);
}

static void *
__new(Object_Cache *cache, char *class_name)
{ 
    Map *map = cache->class_map;
    allocator_t *allocator = ((Obj *)cache)->allocator;
    List *list = NULL; //all same objects in one list;
    List *object_list = cache->object_list;
    Obj *o = NULL;
    int ret = 0;

    /*
     *dbg_str(DBG_DETAIL, "get a object cache");
     *dbg_str(DBG_DETAIL, "map addr:%p", cache->class_map);
     */
    ret = map->search(map, class_name, (void **)&list);
    if (ret != 1) {
        list = object_new(allocator, "Linked_List", NULL);
        if (list == NULL) {
            dbg_str(DBG_ERROR, "get object, new list error");
            return NULL;
        }

        o = object_new(allocator, class_name, NULL);
        if (o == NULL) {
            dbg_str(DBG_ERROR, "get object, new class %s error", class_name);
            return NULL;
        }

        map->add(map, class_name, list);
        object_list->add_back(object_list, o);
        object_list->add_back(object_list, list);
    } else {
        if (list->is_empty(list)) {
            o = object_new(allocator, class_name, NULL);
            if (o == NULL) {
                dbg_str(DBG_ERROR, "get object, new class %s error", class_name);
                return NULL;
            }
            object_list->add_back(object_list, o);
        } else {
            dbg_str(DBG_SUC, "get object from cache");
            list->remove(list, (void **)&o);
        }
    }

    o->cache = cache;

    return o;
}

static class_info_entry_t object_cache_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Object_Cache, construct, __construct),
    Init_Nfunc_Entry(2 , Object_Cache, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Object_Cache, new, __new),
    Init_End___Entry(4 , Object_Cache),
};
REGISTER_CLASS("Object_Cache", object_cache_info);

static int test_object_cache_new()
{
    allocator_t *allocator = allocator_get_default_alloc();
    Object_Cache *cache;
    int ret = 0;
    String *str;
    char *test = "abcdefg";

    cache = object_new(allocator, "Object_Cache", NULL);

    /*get object from cache firstly*/
    str = cache->new(cache, "String");
    if (str == NULL) {
        goto end;
    }

    str->assign(str, test);  

    if (strcmp(str->get_cstr(str), test) == 0) {
        ret = 1;
    } else {
        ret = 0;
    }
    object_destroy(str);

    /*get object from cache secondly*/
    str = cache->new(cache, "String");
    if (str == NULL) {
        goto end;
    }
    object_destroy(str);

end:
    object_destroy(cache);

    return ret;
}

REGISTER_TEST_FUNC(test_object_cache_new);
