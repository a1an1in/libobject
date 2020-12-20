#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/try.h>

typedef struct dlist_node_s {
    struct dlist_node_s *next;
    struct dlist_node_s *prev;
    int d;
} dlist_node_t;

static dlist_node_t *dlist_alloc() 
{
   dlist_node_t *node;

   node = (dlist_node_t *)malloc(sizeof(dlist_node_t));

   node->next = node->prev = node;

   return node;
}

static dlist_node_t *new_dlist_node(int d)
{
   dlist_node_t *node;

   node = (dlist_node_t *)malloc(sizeof(dlist_node_t));

   node->next = node->prev = node;
   node->d = d;

   return node;
}

static void dlist_add(dlist_node_t *new, dlist_node_t *head)
{
    new->next = head->next;
    new->next->prev = new;
    head->next = new;
    new->prev = head;
}

int test_mydlist(TEST_ENTRY *entry)
{
    dlist_node_t *head, *new, *pos;
    int i;

    head = dlist_alloc();

    new = new_dlist_node(1);
    dlist_add(new, head);
    new = new_dlist_node(2);
    dlist_add(new, head);
    new = new_dlist_node(3);
    dlist_add(new, head);
    new = new_dlist_node(4);
    dlist_add(new, head);
    new = new_dlist_node(5);
    dlist_add(new, head);

    pos = head;
    for (i = 0; i < 5; i++) {
        pos = pos->next;
        printf("%d: %d ", i, pos->d);
    }

    printf("\n");

}
REGISTER_TEST_CMD(test_mydlist);
