#include <libobject/core/utils/data_structure/map.h>
#include <libobject/attrib_priority.h>
#include <libobject/mockery/mockery.h>

struct A{
    int a;
    int b;
};

int test_print_context(map_iterator_t *it)
{
    struct A *p = (struct A *)map_get_pointer(it);
    dbg_str(DBG_DETAIL, "a =%d, b=%d", p->a, p->b);
}
int test_map_hash_map(TEST_ENTRY *entry)
{
    map_t *map;
    map_iterator_t it;
    struct A *p;
    allocator_t *allocator = allocator_get_default_instance();
    int ret = 0;

    struct A t1 = {1, 2};
    struct A t2 = {2, 2};
    struct A t3 = {3, 2};
    struct A t4 = {4, 2};

    map = map_alloc(allocator, MAP_TYPE_HASH_MAP);
    map_init(map);

    map_insert(map, (char *)"11", &t1);
    map_insert(map, (char *)"22", &t2);
    map_insert(map, (char *)"55", &t4);
    map_insert(map, (char *)"33", &t3);

    map_search(map, (char *)"33", &it);

    p = (struct A *)map_get_pointer(&it);
    if (p->a == 3 && p->b == 2) {
        ret = 1;
    } else {
        dbg_str(DBG_DETAIL, "a =%d, b=%d", p->a, p->b);
        ret = 0;
    } 

    map_for_each(map, test_print_context);

    map_destroy(map);

    return ret;
}
REGISTER_TEST_FUNC(test_map_hash_map);

int test_map_rbtree_map(TEST_ENTRY *entry)
{
    map_t *map;
    map_iterator_t it;
    struct A *p;
    allocator_t *allocator = allocator_get_default_instance();
    int ret = 0;

    struct A t1 = {1, 2};
    struct A t2 = {2, 2};
    struct A t3 = {3, 2};
    struct A t4 = {4, 2};

    map = map_alloc(allocator, MAP_TYPE_RBTREE_MAP);
    map_init(map);

    map_insert(map, (char *)"11", &t1);
    map_insert(map, (char *)"22", &t2);
    map_insert(map, (char *)"55", &t4);
    map_insert(map, (char *)"33", &t3);

    map_search(map, (char *)"33", &it);

    p = (struct A *)map_get_pointer(&it);
    if (p->a == 3 && p->b == 2) {
        ret = 1;
    } else {
        dbg_str(DBG_DETAIL, "a =%d, b=%d", p->a, p->b);
        ret = 0;
    } 

    map_for_each(map, test_print_context);

    map_destroy(map);

    return ret;
}
REGISTER_TEST_FUNC(test_map_rbtree_map);