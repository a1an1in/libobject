/**
 * @file object.c
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
#include <libobject/core/object.h>

static int __construct(Obj *obj, char *init_str)
{
    dbg_str(OBJ_DETAIL, "obj construct, obj addr:%p", obj);

    return 0;
}

static int __deconstrcut(Obj *obj)
{
    dbg_str(OBJ_DETAIL, "obj deconstruct, obj addr:%p", obj);
    allocator_mem_free(obj->allocator, obj);

    return 0;
}

/**
 * @Synopsis  
 *
 * @Param obj
 * @Param attrib
 *        if you want call this func, must designate class name in the attrib, like the format:"ClassName/attrib_name" to tell the object which class's attrib you want to set.
 * @Param value
 *
 * @Returns   
 */
static int __set(Obj *obj, char *attrib, void *value)
{
    class_info_entry_t * info, *entry;
    class_deamon_t *deamon;
    allocator_t *allocator = obj->allocator;
    uint8_t *base = (uint8_t *)obj;
    int ret = 0;
    char *buf = NULL;  
    char **out = NULL;  
    int cnt;
    char *target_name;

    cnt = compute_slash_count(attrib);
    if (cnt > 0 ) {
        out = allocator_mem_alloc(allocator, sizeof(char *) * cnt);
        if (out == NULL) {
            dbg_str(OBJ_WARNNING, "Obj set alloc err");
            return -1;
        }
        buf = allocator_mem_alloc(allocator, strlen(attrib));
        if (buf == NULL) {
            dbg_str(OBJ_WARNNING, "oss set alloc err");
            return -1;
        }
        strcpy(buf, attrib);
        str_split(buf, "/", out, &cnt);

        dbg_str(DBG_WARNNING, "set class name:%s", out[cnt - 2]);
        target_name = out[cnt - 2];
        attrib = out[cnt - 1];
    } else {
        target_name = obj->target_name;
    }

    deamon = class_deamon_get_global_class_deamon();
    info   = (class_info_entry_t *)
              class_deamon_search_class(deamon,
                                        target_name);

    entry  = __object_get_entry_of_class(info, attrib);
    if (entry == NULL) {
        return -1;
    }

    switch(entry->type) {
        case ENTRY_TYPE_INT8_T:
        case ENTRY_TYPE_UINT8_T:
            {
                uint8_t *addr = (base + entry->offset);
                *addr = *((uint8_t *)value);
                break;
            }
        case ENTRY_TYPE_INT16_T:
        case ENTRY_TYPE_UINT16_T:
            {
                uint16_t *addr = (base + entry->offset);
                *addr = *((uint16_t *)value);
                break;
            }
        case ENTRY_TYPE_INT32_T:
        case ENTRY_TYPE_UINT32_T:
            {
                uint32_t *addr = (base + entry->offset);
                *addr = *((uint32_t *)value);
                break;
            }
        case ENTRY_TYPE_INT64_T:
        case ENTRY_TYPE_UINT64_T:
            {
                uint64_t *addr = (base + entry->offset);
                *addr = *((uint64_t *)value);
                break;
            }
        case ENTRY_TYPE_FLOAT_T:
            break;
        case ENTRY_TYPE_STRING:
            {
                char *addr = (base + entry->offset);
                strcpy(addr, value);
                break;
            }
        case ENTRY_TYPE_NORMAL_POINTER:
        case ENTRY_TYPE_FUNC_POINTER:
        case ENTRY_TYPE_VFUNC_POINTER:
        case ENTRY_TYPE_IFUNC_POINTER:
        case ENTRY_TYPE_OBJ_POINTER:
            {
                void **addr = (base + entry->offset);
                *addr = value;
                break;
            }
        default:
            dbg_str(DBG_DETAIL, "set %s, not support %s item",
                    obj->target_name,
                    attrib);
            ret = -1;
            break;
    }

    if (out != NULL)
        allocator_mem_free(allocator, out);
    if (buf != NULL)
        allocator_mem_free(allocator, buf);

    return ret;
}

/**
 * @Synopsis  
 *
 * @Param obj
 * @Param attrib
 *        if you want call this func, must designate class name in the attrib, like the format:"ClassName/attrib_name" to tell the object which class's attrib you want to get.
 * @Param value
 *
 * @Returns   
 */
static void *__get(Obj *obj, char *attrib)
{
    class_info_entry_t * info, *entry;
    class_deamon_t *deamon;
    uint8_t *base = (uint8_t *)obj;
    allocator_t *allocator = obj->allocator;
    void *addr;
    char *buf = NULL;  
    char **out = NULL;  
    int cnt;
    char *target_name;

    cnt = compute_slash_count(attrib);
    if (cnt > 0 ) {
        out = allocator_mem_alloc(allocator, sizeof(char *) * cnt);
        if (out == NULL) {
            dbg_str(OBJ_WARNNING, "Obj set alloc err");
            return -1;
        }
        buf = allocator_mem_alloc(allocator, strlen(attrib));
        if (buf == NULL) {
            dbg_str(OBJ_WARNNING, "oss set alloc err");
            return -1;
        }
        strcpy(buf, attrib);
        str_split(buf, "/", out, &cnt);

        dbg_str(DBG_WARNNING, "set class name:%s", out[cnt - 2]);
        target_name = out[cnt - 2]; //class name
        attrib = out[cnt - 1]; //real attrib name
    } else {
        target_name = obj->target_name;
    }

    deamon = class_deamon_get_global_class_deamon();
    info   = (class_info_entry_t *)
              class_deamon_search_class(deamon,
                                        target_name);

    entry  = __object_get_entry_of_class(info, attrib);
    if (entry == NULL) {
        return NULL;
    }

    switch(entry->type) {
        case ENTRY_TYPE_INT8_T:
        case ENTRY_TYPE_UINT8_T:
            {
                addr = (base + entry->offset);
                break;
            }
        case ENTRY_TYPE_INT16_T:
        case ENTRY_TYPE_UINT16_T:
            {
                addr = (base + entry->offset);
                break;
            }
        case ENTRY_TYPE_INT32_T:
        case ENTRY_TYPE_UINT32_T:
            {
                addr = (base + entry->offset);
                break;
            }
        case ENTRY_TYPE_INT64_T:
        case ENTRY_TYPE_UINT64_T:
            {
                addr = (base + entry->offset);
                break;
            }
        case ENTRY_TYPE_FLOAT_T:
            break;
        case ENTRY_TYPE_STRING:
            {
                dbg_str(DBG_SUC, "class name:%s get attrib:%s",
                        obj->target_name,
                        attrib);
                addr = (base + entry->offset);
                break;
            }
        case ENTRY_TYPE_NORMAL_POINTER:
        case ENTRY_TYPE_FUNC_POINTER:
        case ENTRY_TYPE_VFUNC_POINTER:
        case ENTRY_TYPE_IFUNC_POINTER:
        case ENTRY_TYPE_OBJ_POINTER:
            {
                addr = (base + entry->offset);
                break;
            }
        default:
            dbg_str(DBG_DETAIL, "get %s, not support %s item",
                    obj->target_name,
                    attrib);
            addr = NULL;
            break;
    }

    if (out != NULL)
        allocator_mem_free(allocator, out);
    if (buf != NULL)
        allocator_mem_free(allocator, buf);
    return addr;
}

static class_info_entry_t obj_class_info[] = {
    [0] = {ENTRY_TYPE_NORMAL_POINTER, "allocator_t", "allocator", NULL, sizeof(void *), offset_of_class(Obj, allocator)}, 
    [1] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *), offset_of_class(Obj, set)}, 
    [2] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *), offset_of_class(Obj, get)}, 
    [3] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *), offset_of_class(Obj, construct)}, 
    [4] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *), offset_of_class(Obj, deconstruct)}, 
    [5] = {ENTRY_TYPE_STRING, "", "name", NULL, sizeof(void *), offset_of_class(Obj, name)}, 
    [6] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Obj", obj_class_info);

void test_obj()
{
    Obj *obj;
    char buf[2048];
    allocator_t *allocator = allocator_get_default_alloc();

    obj = OBJECT_NEW(allocator, Obj, "");

    object_dump(obj, "Obj", buf, 2048);
}


