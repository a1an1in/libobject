#include <stdio.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/mockery/mockery.h>
#include <libobject/core/try.h>

typedef struct slist_node_s {
    struct slist_node_s *next;
    int d;
} slist_node_t;

static slist_node_t *slist_alloc() 
{
   slist_node_t *node;

   node = (slist_node_t *)malloc(sizeof(slist_node_t));

   node->next = NULL;

   return node;
}

static slist_node_t *new_slist_node(int d)
{
   slist_node_t *node;

   node = (slist_node_t *)malloc(sizeof(slist_node_t));

   node->next = NULL;
   node->d = d;

   return node;
}

static void slist_add(slist_node_t *new, slist_node_t *head)
{
    slist_node_t *cur, *tail;

    cur = head;
    while (cur->next != NULL) {
        cur = cur->next;
    }
    
    tail = cur;
    tail->next = new;
}

int test_slist_add(TEST_ENTRY *entry)
{
    slist_node_t *head, *new, *pos;
    int i;

    head = slist_alloc();

    new = new_slist_node(1);
    slist_add(new, head);
    new = new_slist_node(2);
    slist_add(new, head);
    new = new_slist_node(3);
    slist_add(new, head);
    new = new_slist_node(4);
    slist_add(new, head);
    new = new_slist_node(5);
    slist_add(new, head);

    pos = head;
    for (i = 0; pos != NULL; i++) {
        printf("%d: %d ", i, pos->d);
        pos = pos->next;
    }

    printf("\n");

}
REGISTER_TEST_CMD(test_slist_add);

slist_node_t * slist_rotate(slist_node_t *head)
{
    slist_node_t *new_head, *tail, *n;

    new_head = head;
    tail = head;

    while (head != NULL) {
        n = head;
        head = head->next;
        n->next = new_head;
        new_head = n;
    }

    tail->next = NULL;

    return new_head;
}

int test_slist_rotate(TEST_ENTRY *entry)
{
    slist_node_t *head, *new, *pos;
    int i;

    head = slist_alloc();

    new = new_slist_node(1);
    slist_add(new, head);
    new = new_slist_node(2);
    slist_add(new, head);
    new = new_slist_node(3);
    slist_add(new, head);
    new = new_slist_node(4);
    slist_add(new, head);
    new = new_slist_node(5);
    slist_add(new, head);

    pos = head;
    for (i = 0; pos != NULL; i++) {
        printf("%d: %d ", i, pos->d);
        pos = pos->next;
    }
    printf("\n");

    head = slist_rotate(head);
    pos = head;
    for (i = 0; pos != NULL; i++) {
        printf("%d: %d ", i, pos->d);
        pos = pos->next;
    }

    printf("\n");

}
REGISTER_TEST_CMD(test_slist_rotate);

slist_node_t * slist_rotate_n(slist_node_t *head, int c)
{
    slist_node_t *new_head, *tail, *n;
    int count = 0;

    new_head = head;
    tail = head;

    while (head != NULL && count != c) {
        count++;
        n = head;
        head = head->next;
        n->next = new_head;
        new_head = n;
    }

    if (count == c) {
        dbg_str(DBG_DETAIL, "run at here");
        tail->next = head;
    } else {
        dbg_str(DBG_DETAIL, "run at here");
        tail->next = NULL;
    }

    return new_head;
}

int test_slist_rotate_n(TEST_ENTRY *entry)
{
    slist_node_t *head, *new, *pos;
    int i;

    head = slist_alloc();

    new = new_slist_node(1);
    slist_add(new, head);
    new = new_slist_node(2);
    slist_add(new, head);
    new = new_slist_node(3);
    slist_add(new, head);
    new = new_slist_node(4);
    slist_add(new, head);
    new = new_slist_node(5);
    slist_add(new, head);

    pos = head;
    for (i = 0; pos != NULL; i++) {
        printf("%d: %d ", i, pos->d);
        pos = pos->next;
    }
    printf("\n");

    head = slist_rotate_n(head, 2);
    pos = head;
    for (i = 0; pos != NULL; i++) {
        printf("%d: %d ", i, pos->d);
        pos = pos->next;
    }

    printf("\n");

}
REGISTER_TEST_CMD(test_slist_rotate_n);

