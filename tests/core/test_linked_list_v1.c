#include <stdio.h>
#include <string.h>
#include "libobject/core/utils/dbg/debug.h"
#include "libobject/core/utils/data_structure/link_list.h"
#include <libobject/core/utils/registry/registry.h>
#include <libobject/mockery/mockery.h>

struct test{
    int a;
    int b;
};
void print_list_data(struct test *t)
{
    dbg_str(DBG_DETAIL,"a=%d,b=%d",t->a,t->b);
}

static struct test *init_test_instance(struct test *t, int a, int b)
{
    t->a = a;
    t->b = b;

    return t;
}

int test_linked_list_v1_add(TEST_ENTRY *entry)
{
    llist_t *llist;
    allocator_t *allocator = allocator_get_default_instance();
    int lock_type = 1;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret = 0;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);

    llist = llist_alloc(allocator);
    llist_set(llist,"lock_type",&lock_type);
    llist_init(llist);

    llist_add_back(llist, &t1);
    llist_add_back(llist, &t2);
    llist_add_back(llist, &t3);
    llist_add_back(llist, &t4);

    if (llist->list_count == 4) {
        ret = 1;
        goto end;
    } else {
        ret = 0;
        goto end;
    }

end:
    llist_destroy(llist);
    return ret;
}
REGISTER_TEST_FUNC(test_linked_list_v1_add);

int test_linked_list_v1_search(TEST_ENTRY *entry)
{
    llist_t *llist;
    allocator_t *allocator = allocator_get_default_instance();
    int lock_type = 1;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret = 1;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);

    llist = llist_alloc(allocator);
    llist_set(llist,"lock_type",&lock_type);
    llist_init(llist);

    llist_add_front(llist, &t0);
    llist_add_front(llist, &t1);
    llist_add_front(llist, &t2);
    llist_add_front(llist, &t3);

    llist_destroy(llist);
    return ret;
}
REGISTER_TEST_FUNC(test_linked_list_v1_search);

int test_linked_list_v1_remove_front(TEST_ENTRY *entry)
{
    llist_t *llist;
    allocator_t *allocator = allocator_get_default_instance();
    int lock_type = 1;
    struct test *t, t0, t1, t2, t3, t4, t5;
    int ret = 0;

    init_test_instance(&t0, 0, 2);
    init_test_instance(&t1, 1, 2);
    init_test_instance(&t2, 2, 2);
    init_test_instance(&t3, 3, 2);
    init_test_instance(&t4, 4, 2);
    init_test_instance(&t5, 5, 2);

    llist = llist_alloc(allocator);
    llist_set(llist,"lock_type",&lock_type);
    llist_init(llist);

    llist_add_front(llist, &t0);
    llist_add_front(llist, &t1);
    llist_add_front(llist, &t2);
    llist_add_front(llist, &t3);

    llist_remove_front(llist, (void **)&t);
    if (t->a == 3 && t->b == 2) {
        ret = 1;
        goto end;
    } else {
        ret = 0;
        goto end;
    }

end:
    llist_destroy(llist);
    return ret;
}
REGISTER_TEST_FUNC(test_linked_list_v1_remove_front);