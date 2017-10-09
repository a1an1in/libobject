#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <libobject/utils/data_structure/rbtree_map.h>
#include <libobject/utils/dbg/debug.h>

static int timeval_key_cmp_func(void *key1,void *key2,uint32_t size)
{
    struct timeval *k1, *k2;

    k1 = (struct timeval *) key1;
    k2 = (struct timeval *) key2;

    
    if (    k1->tv_sec > k2->tv_sec || 
            (k1->tv_sec == k2->tv_sec && k1->tv_usec > k2->tv_usec))
    {
        return 1;
    } else if (k1->tv_sec == k2->tv_sec && k1->tv_usec == k2->tv_usec) {
        return 0;
    } else {
        return -1;
    }
}

static int numeric_key_cmp_func(void *key1,void *key2,uint32_t size)
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


int test_datastructure_rbtree_map_string_key(void)
{
    int ret = 0;
    rbtree_map_pos_t it,next,end;
    rbtree_map_t *map;
    allocator_t *allocator = allocator_get_default_alloc();
    pair_t *pair;
    struct rbtree_map_node *mnode;
    int key_len = 2;

    struct test *t, t0, t1, t2, t3, t4, t5;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);

    dbg_str(DBG_DETAIL,"rbtree allocator addr:%p",allocator);
    map = rbtree_map_alloc(allocator);

    rbtree_map_init(map,key_len,sizeof(void *)); 


    rbtree_map_insert(map,(char *)"00", &t0);
    rbtree_map_insert(map,(char *)"11", &t1);
    rbtree_map_insert(map,(char *)"22", &t2);
    rbtree_map_insert(map,(char *)"33", &t3);


    dbg_str(DBG_DETAIL,"foreach ordinal print");
    for(    rbtree_map_begin(map,&it),rbtree_map_pos_next(&it,&next); 
            !rbtree_map_pos_equal(&it,rbtree_map_end(map,&end));
            it = next,rbtree_map_pos_next(&it,&next))
    {
        mnode = rb_entry(it.rb_node_p,struct rbtree_map_node,node);
        dbg_buf(DBG_DETAIL,"key:",mnode->key,mnode->value_pos);
        t = rbtree_map_pos_get_pointer(&it);
        dbg_str(DBG_DETAIL,"t->a=%d t->b=%d", t->a, t->b);
    }

    dbg_str(DBG_DETAIL,"search node key = 22");
    rbtree_map_search(map, (void *)"22",&it);
    t = rbtree_map_pos_get_pointer(&it);
    dbg_str(DBG_DETAIL,"t->a=%d t->b=%d", t->a, t->b);

    rbtree_map_destroy(map);
    /*
     *allocator_destroy(allocator);
     */
    dbg_str(DBG_DETAIL,"map addr =%p",map);

    return ret;
}

int test_datastructure_rbtree_map_numeric_key(void)
{
    int ret = 0;
    rbtree_map_pos_t it,next,end;
    rbtree_map_t *map;
    allocator_t *allocator = allocator_get_default_alloc();
    pair_t *pair;
    struct rbtree_map_node *mnode;
    int key_len = 4;
    int key;

    struct test *t, t0, t1, t2, t3, t4, t5;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);

    dbg_str(DBG_DETAIL,"rbtree allocator addr:%p",allocator);
    map = rbtree_map_alloc(allocator);

    map->key_cmp_func = numeric_key_cmp_func;

    rbtree_map_init(map,key_len,sizeof(void *)); 

    rbtree_map_insert_with_numeric_key(map,0, &t0);
    rbtree_map_insert_with_numeric_key(map,1, &t1);
    rbtree_map_insert_with_numeric_key(map,2, &t2);
    rbtree_map_insert_with_numeric_key(map,3, &t3);


    dbg_str(DBG_DETAIL,"foreach ordinal print");
    for(    rbtree_map_begin(map,&it),rbtree_map_pos_next(&it,&next); 
            !rbtree_map_pos_equal(&it,rbtree_map_end(map,&end));
            it = next,rbtree_map_pos_next(&it,&next))
    {
        mnode = rb_entry(it.rb_node_p,struct rbtree_map_node,node);
        dbg_buf(DBG_DETAIL,"key:",mnode->key,mnode->value_pos);
        t = rbtree_map_pos_get_pointer(&it);
        dbg_str(DBG_DETAIL,"t->a=%d t->b=%d", t->a, t->b);
    }

    dbg_str(DBG_DETAIL,"search node key = 3");
    rbtree_map_search_by_numeric_key(map, 3,&it);
    t = rbtree_map_pos_get_pointer(&it);
    dbg_str(DBG_DETAIL,"t->a=%d t->b=%d", t->a, t->b);

    rbtree_map_destroy(map);
    dbg_str(DBG_DETAIL,"map addr =%p",map);

    return ret;
}

int test_datastructure_rbtree_map_timeval_key(void)
{
    int ret = 0;
    rbtree_map_pos_t it,next,end;
    rbtree_map_t *map;
    allocator_t *allocator = allocator_get_default_alloc();
    pair_t *pair;
    struct rbtree_map_node *mnode;
    int key_len = sizeof(struct timeval);
    int key;
    struct timeval tp, t_bak, tv;
    struct test *t, t0, t1, t2, t3, t4, t5;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);

    dbg_str(DBG_DETAIL,"rbtree allocator addr:%p, key_len =%d",allocator,key_len);
    map = rbtree_map_alloc(allocator);

    map->key_cmp_func = timeval_key_cmp_func;
    map->fixed_key_len = 1;

    rbtree_map_init(map,key_len,sizeof(void *)); 

    gettimeofday(&tp, NULL);
    printf("time %ld:%ld\n",tp.tv_sec,tp.tv_usec);
    dbg_buf(DBG_DETAIL,"key:", (void *)&tp, key_len);
    rbtree_map_insert(map,&tp, &t0);

    gettimeofday(&tp, NULL);
    printf("time %ld:%ld\n",tp.tv_sec,tp.tv_usec);
    dbg_buf(DBG_DETAIL,"key:", (void *)&tp, key_len);
    t_bak = tp;
    rbtree_map_insert(map,&tp, &t1);

    gettimeofday(&tp, NULL);
    printf("time %ld:%ld\n",tp.tv_sec,tp.tv_usec);
    dbg_buf(DBG_DETAIL,"key:", (void *)&tp, key_len);
    rbtree_map_insert(map,&tp, &t2);

    gettimeofday(&tp, NULL);
    printf("time %ld:%ld\n",tp.tv_sec,tp.tv_usec);
    dbg_buf(DBG_DETAIL,"key:",(void *)&tp, key_len);
    rbtree_map_insert(map,&tp, &t3);


    dbg_str(DBG_DETAIL,"foreach ordinal print");
    for(    rbtree_map_begin(map,&it),rbtree_map_pos_next(&it,&next); 
            !rbtree_map_pos_equal(&it,rbtree_map_end(map,&end));
            it = next,rbtree_map_pos_next(&it,&next))
    {
        mnode = rb_entry(it.rb_node_p,struct rbtree_map_node,node);
        dbg_buf(DBG_DETAIL,"key:",mnode->key,mnode->value_pos);
        t = rbtree_map_pos_get_pointer(&it);
        dbg_str(DBG_DETAIL,"t->a=%d t->b=%d", t->a, t->b);
    }

    dbg_str(DBG_DETAIL,"search node key = 3");
    dbg_buf(DBG_DETAIL,"key:", (void *)&t_bak, key_len);
    rbtree_map_search(map, &t_bak,&it);
    t = rbtree_map_pos_get_pointer(&it);
    dbg_str(DBG_DETAIL,"t->a=%d t->b=%d", t->a, t->b);

    rbtree_map_destroy(map);
    dbg_str(DBG_DETAIL,"map addr =%p",map);

    return ret;
}

int test_datastructure_rbtree_map(void)
{
    test_datastructure_rbtree_map_string_key();
    /*
     *test_datastructure_rbtree_map_numeric_key();
     */
    /*
     *test_datastructure_rbtree_map_timeval_key();
     */

}
