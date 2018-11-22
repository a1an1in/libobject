/**
 * @file map.c
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
#include <libobject/core/utils/config/config.h>
#include <libobject/core/rbtree_map.h>
#include <libobject/core/linked_list.h>

static int __find_same_key_node(struct rb_node *node, key_cmp_fpt func, void *key, List *list)
{
    int ret = 0;
    void **addr;

    while (node) {
        struct rbtree_map_node *mnode = rb_entry(node, struct rbtree_map_node, node);
        int result;

        result = func(key, mnode->key, mnode->value_pos);
        if (result < 0) {
            node = node->rb_left;
        } else if (result > 0) {
            node = node->rb_right;
        } else {
            addr  = (void **)&mnode->key[mnode->value_pos];
            dbg_str(DBG_DETAIL, "list add a element:%p", *addr);
            list->add(list, *addr);
            __find_same_key_node(node->rb_left, func , key, list);
            __find_same_key_node(node->rb_right, func,  key, list);
            break;
        }
    }

    return ret;
}

static int __construct(Map *map, char *init_str)
{
    RBTree_Map *rbm = (RBTree_Map *)map;
    allocator_t *allocator = map->obj.allocator;

    if (rbm->key_size == 0)    { rbm->key_size = sizeof(void *);}
    if (rbm->value_size == 0)  { rbm->value_size = sizeof(void *); }

    dbg_str(DBG_DETAIL, "rbtree map construct, key_size=%d, value_size=%d, key_cmp_cb=%p", 
            rbm->key_size, rbm->value_size, rbm->key_cmp_cb);

    rbm->rbmap = rbtree_map_alloc(allocator);

    rbtree_map_init(rbm->rbmap, rbm->key_size, rbm->value_size); 

    map->b = OBJECT_NEW(allocator, RBTree_Iterator, NULL);
    map->e = OBJECT_NEW(allocator, RBTree_Iterator, NULL);

    return 0;
}

static int __deconstrcut(Map *map)
{
    dbg_str(DBG_DETAIL, "hash map deconstruct, map addr:%p", map);
    object_destroy(map->b);
    object_destroy(map->e);
    rbtree_map_destroy(((RBTree_Map *)map)->rbmap);

    return 0;
}

static int __set(Map *m, char *attrib, void *value)
{
    RBTree_Map *map = (RBTree_Map *)m;

    if (strcmp(attrib, "set") == 0) {
        map->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        map->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        map->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        map->deconstruct = value;
    } else if (strcmp(attrib, "name") == 0) {
        strncpy(map->name, value, strlen(value));
    } else if (strcmp(attrib, "add") == 0) {
        map->add = value;
    } else if (strcmp(attrib, "search") == 0) {
        map->search = value;
    } else if (strcmp(attrib, "search_all_same_key") == 0) {
        map->search_all_same_key = value;
    } else if (strcmp(attrib, "remove") == 0) {
        map->remove = value;
    } else if (strcmp(attrib, "del") == 0) {
        map->del = value;
    } else if (strcmp(attrib, "for_each") == 0) {
        map->for_each = value;
    } else if (strcmp(attrib, "begin") == 0) {
        map->begin = value;
    } else if (strcmp(attrib, "end") == 0) {
        map->end = value;
    } else if (strcmp(attrib, "destroy") == 0) {
        map->destroy = value;
    } else if (strcmp(attrib, "key_size") == 0) {
        map->key_size = *((uint16_t *)value);
    } else if (strcmp(attrib, "value_size") == 0) {
        map->value_size = *((uint16_t *)value);
    } else if (strcmp(attrib, "key_type") == 0) {
        map->key_type = *((uint8_t *)value);
    } else if (strcmp(attrib, "key_cmp_cb") == 0) {
        map->key_cmp_cb = value;
    } else {
        dbg_str(RBTMAP_WARNNING, "map set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Map *obj, char *attrib)
{
    RBTree_Map *map = (RBTree_Map *)obj;

    if (strcmp(attrib, "name") == 0) {
        return map->name;
    } else if (strcmp(attrib, "key_size") == 0) {
        return &map->key_size;
    } else if (strcmp(attrib, "value_size") == 0) {
        return &map->value_size;
    } else if (strcmp(attrib, "key_type") == 0) {
        return &map->key_type;
    } else {
        dbg_str(RBTMAP_WARNNING, "hash map get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }

    return NULL;
}

static int __add(Map *map, void *key, void *value)
{
    dbg_str(DBG_DETAIL, "Rbtree Map add");
    RBTree_Map *rbmap = (RBTree_Map *)map;

    if (rbmap->key_type) {
        rbmap->rbmap->key_type = rbmap->key_type;
    }

    return rbtree_map_insert(rbmap->rbmap, key, value);
}

static int __search(Map *map, void *key, void **element)
{
    rbtree_map_pos_t pos;
    int ret;

    dbg_str(RBTMAP_IMPORTANT, "Rbtree Map search");
    ret = rbtree_map_search(((RBTree_Map *)map)->rbmap, key, &pos);
    if (ret == 1 && element != NULL) {
        *element = rbtree_map_pos_get_pointer(&pos);
        return ret;
    }

    return ret;
}

static int __search_all_same_key(Map *map, void *key, List *list)
{
    rbtree_map_t *rbtree_map = ((RBTree_Map *)map)->rbmap;
    struct rb_root *root = rbtree_map->tree_root;
    struct rb_node *node = root->rb_node;
    int ret = 0;
    void **addr;

    dbg_str(DBG_DETAIL, "rbtree_map_search all");

    sync_lock(&rbtree_map->map_lock, NULL);
    key_cmp_fpt key_cmp_func = rbtree_map->key_cmp_func;

    while (node) {
        struct rbtree_map_node *mnode = rb_entry(node, struct rbtree_map_node, node);
        int result;

        result = key_cmp_func(key, mnode->key, mnode->value_pos);
        if (result < 0) {
            node = node->rb_left;
        } else if (result > 0) {
            node = node->rb_right;
        } else {
            addr  = (void **)&mnode->key[mnode->value_pos];
            dbg_str(DBG_DETAIL, "list add a element:%p", *addr);
            list->add(list, *addr);
            __find_same_key_node(node->rb_left, key_cmp_func , key, list);
            __find_same_key_node(node->rb_right, key_cmp_func,  key, list);
            break;
        }
    }
    sync_unlock(&rbtree_map->map_lock);

    return ret;
}

static int __remove(Map *map, void *key, void **element)
{
    rbtree_map_pos_t pos;
    int ret;

    dbg_str(RBTMAP_IMPORTANT, "Rbtree Map remove");
    ret = rbtree_map_search(((RBTree_Map *)map)->rbmap, key, &pos);
    if (ret == 1 && element != NULL) {
        *element = rbtree_map_pos_get_pointer(&pos);
        ret = rbtree_map_delete(((RBTree_Map *)map)->rbmap, &pos);
        return ret;
    } else if (ret == 1) {
        ret = rbtree_map_delete(((RBTree_Map *)map)->rbmap, &pos);
    } else {
        dbg_str(RBTMAP_WARNNING, "map remove, not found key :%s", key);
    }

    return ret;
}

static int __del(Map *map, void *key)
{
    rbtree_map_pos_t pos;
    int ret = -1;
    dbg_str(DBG_DETAIL, "Rbtree Map del");

    ret = rbtree_map_search(((RBTree_Map *)map)->rbmap, key, &pos);
    if (ret == 1) {
        ret = rbtree_map_delete(((RBTree_Map *)map)->rbmap, &pos);
    }

    return ret;
}

static Iterator *__begin(Map *map)
{
    RBTree_Iterator *iter = (RBTree_Iterator *)map->b;

    dbg_str(DBG_DETAIL, "Rbtree Map begin");

    rbtree_map_begin(((RBTree_Map *)map)->rbmap, &iter->rbtree_map_pos);

    return (Iterator *)iter;
}

static Iterator *__end(Map *map)
{
    RBTree_Iterator *iter = (RBTree_Iterator *)map->e;

    dbg_str(DBG_DETAIL, "Rbtree Map end");

    rbtree_map_end(((RBTree_Map *)map)->rbmap, &iter->rbtree_map_pos);

    return (Iterator *)iter;
}

static class_info_entry_t rbtree_map_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ, "Map", "map", NULL, sizeof(void *)}, 
    [1 ] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2 ] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3 ] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4 ] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "add", __add, sizeof(void *)}, 
    [6 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "search", __search, sizeof(void *)}, 
    [7 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "search_all_same_key", __search_all_same_key, sizeof(void *)}, 
    [8 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "remove", __remove, sizeof(void *)}, 
    [9 ] = {ENTRY_TYPE_VFUNC_POINTER, "", "del", __del, sizeof(void *)}, 
    [10] = {ENTRY_TYPE_VFUNC_POINTER, "", "begin", __begin, sizeof(void *)}, 
    [11] = {ENTRY_TYPE_VFUNC_POINTER, "", "end", __end, sizeof(void *)}, 
    [12] = {ENTRY_TYPE_VFUNC_POINTER, "", "for_each", NULL, sizeof(void *)}, 
    [13] = {ENTRY_TYPE_UINT16_T, "", "key_size", NULL, sizeof(short)}, 
    [14] = {ENTRY_TYPE_UINT16_T, "", "value_size", NULL, sizeof(short)}, 
    [15] = {ENTRY_TYPE_UINT8_T, "", "key_type", NULL, sizeof(short)}, 
    [16] = {ENTRY_TYPE_NORMAL_POINTER, "", "key_cmp_cb", NULL, sizeof(short)}, 
    [17] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("RBTree_Map", rbtree_map_class_info);

static int numeric_key_cmp_func(void *key1, void *key2, uint32_t size)
{
    int k1, k2;

    k1 = *((int *)key1);
    k2 = *((int *)key2);

    if (k1 > k2 ) return 1;
    else if (k1 < k2) return -1;
    else return 0;
}

struct test{
    int a;
    int b;
};
static struct test *init_test_instance(struct test *t, int a, int b)
{
    t->a = a;
    t->b = b;

    return t;
}

static void rbtree_map_print(void *key, void *element)
{
    struct test *t = (struct test *)element;
     
    dbg_str(DBG_DETAIL, "key:%s t->a=%d, t->b=%d", key, t->a, t->b);
}

static void rbtree_map_print_with_numeric_key(void *key, void *element)
{
    struct test *t = (struct test *)element;
     
    dbg_str(DBG_DETAIL, "key:%d t->a=%d, t->b=%d", *(int *)key, t->a, t->b);
}


void test_obj_rbtree_map_string_key()
{
    Iterator *iter, *next, *prev;
    Map *map;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    cjson_t *root, *e, *s;
    char buf[2048] = {0};
    char set_str[2048] = {0};
    struct test *t, t0, t1, t2, t3, t4, t5;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);

    dbg_str(RBTMAP_SUC, "rbtree_map test begin alloc count =%d", allocator->alloc_count);

    c = cfg_alloc(allocator); 
    dbg_str(RBTMAP_SUC, "configurator_t addr:%p", c);
    cfg_config(c, "/RBTree_Map", CJSON_NUMBER, "key_size", "40") ;  
    cfg_config(c, "/RBTree_Map", CJSON_NUMBER, "value_size", "8") ;
    cfg_config(c, "/RBTree_Map", CJSON_NUMBER, "key_cmp_cb", "43") ;

    map  = OBJECT_NEW(allocator, RBTree_Map, c->buf);

    object_dump(map, "RBTree_Map", buf, 2048);
    dbg_str(DBG_DETAIL, "Map dump: %s", buf);

    map->add(map, "test0", &t0);
    map->add(map, "test1", &t1);
    map->add(map, "test2", &t2);
    map->add(map, "test3", &t3);
    map->add(map, "test4", &t4);
    map->add(map, "test5", &t5);
    map->for_each(map, rbtree_map_print);

    dbg_str(DBG_DETAIL, "test search:");
    map->search(map, "test2", (void **)&t);
    dbg_str(DBG_DETAIL, "new search test2: t->a=%d, t->b=%d", t->a, t->b);

    /*
     *dbg_str(DBG_DETAIL, "test del:");
     *map->del(map, "test2");
     */

    dbg_str(DBG_DETAIL, "test remove:");
    map->remove(map, "test2", (void **)&t);
    dbg_str(DBG_DETAIL, "remove test2: t->a=%d, t->b=%d", t->a, t->b);

    map->for_each(map, rbtree_map_print);

    object_destroy(map);

    cfg_destroy(c);

    dbg_str(RBTMAP_SUC, "rbtree_map test end alloc count =%d", allocator->alloc_count);

}

void test_obj_rbtree_map_numeric_key()
{
    Iterator *iter, *next, *prev;
    Map *map;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    cjson_t *root, *e, *s;
    char buf[2048] = {0};
    char set_str[2048] = {0};
    int key;
    int ret;
    struct test *t, t0, t1, t2, t3, t4, t5;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);

    dbg_str(RBTMAP_SUC, "rbtree_map test begin alloc count =%d", allocator->alloc_count);

    c = cfg_alloc(allocator); 
    dbg_str(RBTMAP_SUC, "configurator_t addr:%p", c);
    cfg_config(c, "/RBTree_Map", CJSON_NUMBER, "key_type", "1");

    map  = OBJECT_NEW(allocator, RBTree_Map, c->buf);

    object_dump(map, "RBTree_Map", buf, 2048);
    dbg_str(DBG_DETAIL, "Map dump: %s", buf);

    printf("add\n");
    key = 0;
    map->add(map, &key, &t0);
    key = 1;
    map->add(map, &key, &t1);

    printf("for_each\n");
    map->for_each(map, rbtree_map_print_with_numeric_key);

    printf("search\n");
    ret = map->search(map, &key, (void **)&t);
    dbg_str(DBG_DETAIL, "search ret=%d", ret);
    dbg_str(DBG_DETAIL, "search key=%d, t->a=%d, t->b=%d", key, t->a, t->b);

    printf("del\n");
    map->del(map, &key);

    printf("for_each\n");
    map->for_each(map, rbtree_map_print_with_numeric_key);

    object_destroy(map);
    cfg_destroy(c);

    dbg_str(RBTMAP_SUC, "rbtree_map test end alloc count =%d", allocator->alloc_count);

}
void test_obj_rbtree_map()
{
    test_obj_rbtree_map_string_key();
    /*
     *test_obj_rbtree_map_numeric_key();
     */
}

static void test_list_print(void *element)
{
    dbg_str(DBG_DETAIL, "list element value: %p", element);
}

int test_rbtree_map_same_string_key()
{
    Iterator *iter, *next, *prev;
    Map *map;
    List *list;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    cjson_t *root, *e, *s;
    char buf[2048] = {0};
    char set_str[2048] = {0};
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret = 0;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);

    c = cfg_alloc(allocator); 
    dbg_str(RBTMAP_SUC, "configurator_t addr:%p", c);
    cfg_config(c, "/RBTree_Map", CJSON_NUMBER, "key_size", "40") ;  
    cfg_config(c, "/RBTree_Map", CJSON_NUMBER, "value_size", "8") ;
    cfg_config(c, "/RBTree_Map", CJSON_NUMBER, "key_cmp_cb", "43") ;

    map  = OBJECT_NEW(allocator, RBTree_Map, c->buf);
    list = OBJECT_NEW(allocator, Linked_List, NULL);

    map->add(map, "test0", &t0);
    map->add(map, "test1", &t1);
    map->add(map, "test2", &t2);
    map->add(map, "test2", &t2);
    map->add(map, "test2", &t2);
    map->add(map, "test2", &t2);
    map->add(map, "test3", &t3);
    map->add(map, "test4", &t4);
    map->add(map, "test5", &t5);
    map->for_each(map, rbtree_map_print);

    dbg_str(DBG_DETAIL, "run at here");
    map->search_all_same_key(map, "test2", list);
    dbg_str(DBG_DETAIL, "run at here");
    list->for_each(list, test_list_print);

    if (list->count(list) == 4) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(list);
    object_destroy(map);

    cfg_destroy(c);
    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_same_string_key);

int test_rbtree_map_same_numeric_key()
{
    Iterator *iter, *next, *prev;
    Map *map;
    List *list;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    cjson_t *root, *e, *s;
    char buf[2048] = {0};
    char set_str[2048] = {0};
    int key;
    int ret;
    struct test *t, t0, t1, t2, t3, t4, t5;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 1, 2);
    init_test_instance(&t3, 1, 2);
    init_test_instance(&t4, 1, 2);

    c = cfg_alloc(allocator); 
    dbg_str(RBTMAP_SUC, "configurator_t addr:%p", c);
    cfg_config(c, "/RBTree_Map", CJSON_NUMBER, "key_type", "1");

    map  = OBJECT_NEW(allocator, RBTree_Map, c->buf);
    list = OBJECT_NEW(allocator, Linked_List, NULL);

    key = 0;
    map->add(map, &key, &t0);
    key = 1;
    map->add(map, &key, &t1);
    map->add(map, &key, &t1);
    map->add(map, &key, &t1);
    key = 2;
    map->add(map, &key, &t2);
    key = 3;
    map->add(map, &key, &t3);

    printf("for_each\n");
    map->for_each(map, rbtree_map_print_with_numeric_key);

    key = 1;
    map->search_all_same_key(map, &key, list);
    dbg_str(DBG_DETAIL, "run at here");
    list->for_each(list, test_list_print);

    if (list->count(list) == 3) {
        ret = 1;
    } else {
        ret = 0;
    }

    object_destroy(list);
    object_destroy(map);
    cfg_destroy(c);

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_same_numeric_key);

int test_rbtree_map_a_numeric_key()
{
    Iterator *iter, *next, *prev;
    Map *map;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    cjson_t *root, *e, *s;
    char buf[2048] = {0};
    char set_str[2048] = {0};
    int key;
    int ret;
    struct test *t, t0, t1, t2, t3, t4, t5;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 1, 2);
    init_test_instance(&t3, 1, 2);
    init_test_instance(&t4, 1, 2);

    c = cfg_alloc(allocator); 
    dbg_str(RBTMAP_SUC, "configurator_t addr:%p", c);
    cfg_config(c, "/RBTree_Map", CJSON_NUMBER, "key_type", "1");

    map  = OBJECT_NEW(allocator, RBTree_Map, c->buf);

    key = 0;
    map->add(map, &key, &t0);
    key = 1;
    map->add(map, &key, &t1);
    key = 2;
    map->add(map, &key, &t2);
    key = 3;
    map->add(map, &key, &t3);

    printf("for_each\n");
    map->for_each(map, rbtree_map_print_with_numeric_key);

    key = 1;
    ret = map->search(map, &key, (void **)&t);
    if (ret == 1) {
        dbg_str(DBG_SUC, "found key =%d", key);
    }

    key = 2;
    ret = map->search(map, &key, (void **)&t);
    if (ret == 1) {
        dbg_str(DBG_SUC, "found key =%d", key);
    }

    key = 3;
    ret = map->search(map, &key, (void **)&t);
    if (ret == 1) {
        dbg_str(DBG_SUC, "found key =%d", key);
    }

    object_destroy(map);
    cfg_destroy(c);

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_a_numeric_key);



