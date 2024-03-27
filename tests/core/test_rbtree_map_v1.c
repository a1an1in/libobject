#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <libobject/core/utils/data_structure/rbtree_map.h>
#include <libobject/core/try.h>
#include <libobject/mockery/mockery.h>

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


int test_rbtree_map_v1_insert(TEST_ENTRY *entry)
{
    int ret = 0;
    rbtree_map_pos_t it,next,end;
    rbtree_map_t *map;
    allocator_t *allocator = allocator_get_default_instance();
    struct rbtree_map_node *mnode;
    int key_len = 2;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int count = 4, map_count;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);

    map = rbtree_map_alloc(allocator);
    
    rbtree_map_init(map); 

    rbtree_map_insert(map,(char *)"00", &t0);
    rbtree_map_insert(map,(char *)"11", &t1);
    rbtree_map_insert(map,(char *)"22", &t2);
    rbtree_map_insert(map,(char *)"33", &t3);

    map_count = rbtree_map_count(map);

    dbg_str(DBG_DETAIL,"count =%d map_count=%d", count, map_count);
    ret = assert_equal(&map_count, &count, sizeof(count));
    if (ret == 0) {
        goto end;
    }

end:
    rbtree_map_destroy(map);

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_v1_insert);

int test_rbtree_map_v1_search_default(TEST_ENTRY *entry)
{
    int ret = 0;
    rbtree_map_pos_t it,next,end;
    rbtree_map_t *map;
    allocator_t *allocator = allocator_get_default_instance();
    struct rbtree_map_node *mnode;
    struct test *t, t0, t1, t2, t3, t4, t5;
    void *key0 = 0, *key1 = 1, *key2 = 2, *key3 = 3, *key4 = 4, *key5 =5,
        *key6 = 3, *key7 = 7;
    int key_len = sizeof(key0);

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);

    dbg_str(DBG_DETAIL,"rbtree allocator addr:%p",allocator);
    map = rbtree_map_alloc(allocator);
    rbtree_map_init(map); 

    rbtree_map_insert(map, key0, &t0);
    rbtree_map_insert(map, key1, &t1);
    rbtree_map_insert(map, key2, &t2);
    rbtree_map_insert(map, key3, &t3);
    rbtree_map_insert(map, key4, &t4);
    rbtree_map_insert(map, key5, &t5);

    ret = rbtree_map_search(map, key6, &it);
    if (ret == 1) {
        t = rbtree_map_pos_get_pointer(&it);
        dbg_str(DBG_INFO,"t->a=%d t->b=%d", t->a, t->b);
        ret = assert_equal(t, &t2, sizeof(void *));
    } else if (ret == 0){
        goto end;
    }

    ret = rbtree_map_search(map, key7, &it);
    if (ret == 1){
        ret = 0;
        goto end;
    } else {
        ret = 1;
    }

end:
    rbtree_map_destroy(map);

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_v1_search_default);

int test_rbtree_map_v1_search_string_key(TEST_ENTRY *entry)
{
    int ret = 0;
    rbtree_map_pos_t it,next,end;
    rbtree_map_t *map;
    allocator_t *allocator = allocator_get_default_instance();
    struct rbtree_map_node *mnode;
    struct test *t, t0, t1, t2, t3, t4, t5;

    TRY{
        init_test_instance(&t0, 0, 2);
        init_test_instance(&t1, 1, 2);
        init_test_instance(&t2, 2, 2);
        init_test_instance(&t3, 3, 2);
        init_test_instance(&t4, 4, 2);
        init_test_instance(&t5, 5, 2);

        dbg_str(DBG_DETAIL,"rbtree allocator addr:%p",allocator);
        map = rbtree_map_alloc(allocator);
        rbtree_map_set(map, "key_cmp_func", (void *)string_key_cmp_func);
        rbtree_map_init(map); 

        rbtree_map_insert(map,(char *)"00", &t0);
        rbtree_map_insert(map,(char *)"11", &t1);
        rbtree_map_insert(map,(char *)"22", &t2);
        rbtree_map_insert(map,(char *)"33", &t3);

        dbg_str(DBG_DETAIL,"search node key = 22");
        rbtree_map_search(map, (void *)"22",&it);
        t = rbtree_map_pos_get_pointer(&it);
        dbg_str(DBG_INFO,"t->a=%d t->b=%d", t->a, t->b);
        THROW_IF(t != &t2, -1);

        ret = rbtree_map_search(map, (void *)"223",&it);
        THROW_IF(ret == 1, -1);
    } CATCH (ret) {} FINALLY {
        rbtree_map_destroy(map);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_rbtree_map_v1_search_string_key);