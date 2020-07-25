/**
 * @file Trie.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2020-02-19
 */

#include <libobject/core/Trie.h>

static int __delete_node(Trie *trie, trie_node_t *node)
{
    allocator_t *allocator = trie->parent.allocator;
    int count, i, delete_node_count = 0;

    if (node == NULL) return 0;

    count = trie->node_count;

    for (i = 0; i < count; i++) {
        if (node->table[i] == NULL) {
            continue;
        }
        __delete_node(trie, node->table[i]);
        allocator_mem_free(allocator, node->table[i]);
        delete_node_count++;
    }

    allocator_mem_free(allocator, node->table);

    return delete_node_count;
}

static trie_node_t *__new_node(Trie *trie) 
{
    allocator_t *allocator = trie->parent.allocator;
    trie_node_t *node;

    node = allocator_mem_alloc(allocator, sizeof(trie_node_t));
    if (node == NULL) {
        dbg_str(DBG_ERROR, "trie new node error");
        return NULL;
    }
    node->end = 0;

    node->table = allocator_mem_zalloc(allocator,
                                       sizeof(void *) * trie->node_count);
    if (node->table == NULL) {
        dbg_str(DBG_ERROR, "trie new node table error");
        return NULL;
    }

    return node;
}

static int __get_char_index(Trie *trie, char c)
{
    int len, i;

    len = strlen(trie->char_set);

    for (i = 0; i < len; i++) {
        if (trie->char_set[i] == c) {
            return i;
        }
    }

    return -1;
}

static int __construct(Trie *trie, char *init_str)
{
    allocator_t *allocator = trie->parent.allocator;
    char *s = "abcdefghijklmnopqrstuvwxyz";
    int len;

    if (trie->char_set == NULL) {
       len = strlen(s);
       trie->char_set = allocator_mem_alloc(allocator, len);
       strcpy(trie->char_set, s);
    }

    if (trie->node_count == 0) {
        trie->node_count = strlen(trie->char_set);
    }

    trie->root = __new_node(trie);

    return 0;
}

static int __deconstruct(Trie *trie)
{
    allocator_t *allocator = trie->parent.allocator;

    __delete_node(trie, trie->root);
    allocator_mem_free(allocator, trie->root);
    allocator_mem_free(allocator, trie->char_set);

    return 0;
}

static int __set_char_set(Trie *trie, char *word)
{
    allocator_t *allocator = trie->parent.allocator;
    int len, w_len;

    len = strlen(trie->char_set);
    w_len = strlen(word);

    if (w_len > len) {
        allocator_mem_free(allocator, trie->char_set);
        trie->char_set = allocator_mem_alloc(allocator, w_len);
    }

    strcpy(trie->char_set, word);

    return 0;
}

static int __insert(Trie *trie, char *word)
{
    int len, i, index;
    trie_node_t *node;
    allocator_t *allocator = trie->parent.allocator;

    dbg_str(DBG_DETAIL, "trie insert word:%s", word);

    len = strlen(word);
    node = trie->root;

    for (i = 0; i < len; i++) {
        index =  __get_char_index(trie, word[i]);
        if (index < 0) return -1;

        if (node->table[index] == NULL) {
            node->table[index] = __new_node(trie);
            if (node->table[index] == NULL) {
                dbg_str(DBG_ERROR, "trie node alloc error");
                return -1;
            }
        }
        node = node->table[index];
        node->pass++;
    }

    return ++node->end;
}

static int __search(Trie *trie, char *word)
{
    int len, i, index;
    trie_node_t *root, *node;
    allocator_t *allocator = trie->parent.allocator;

    len = strlen(word);
    node = trie->root;

    for (i = 0; i < len; i++) {
        index =  __get_char_index(trie, word[i]);
        if (index < 0) return -1;

        node = node->table[index];
        if (node == NULL) {
            return 0;
        }
    }

    return node->end;
}

static int __search_prefix(Trie *trie, char *word)
{
    int len, i, index;
    trie_node_t *root, *node;
    allocator_t *allocator = trie->parent.allocator;

    len = strlen(word);
    node = trie->root;

    for (i = 0; i < len; i++) {
        index =  __get_char_index(trie, word[i]);
        if (index < 0) return -1;

        node = node->table[index];
        if (node == NULL) {
            return 0;
        }
    }

    return node->pass;
}

static int __delete(Trie *trie, char *word)
{
    int len, i, index;
    trie_node_t *root, *node;
    allocator_t *allocator = trie->parent.allocator;
    int ret;

    len = strlen(word);
    root = trie->root;
    node = trie->root;

    for (i = 0; i < len; i++) {
        index =  __get_char_index(trie, word[i]);
        if (index < 0) return -1;

        node = node->table[index];
        if (node == NULL) {
            return 0;
        }

        if (--node->pass > 0) {
            continue;
        }

        ret = __delete_node(trie, node);
        if (ret > 1) {
            dbg_str(DBG_ERROR, "delete word, but delete %d node", ret);
        }
        node->table[index] = NULL;
    }

    return --node->end;
}

static class_info_entry_t trie_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Trie, construct, __construct),
    Init_Nfunc_Entry(2, Trie, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Trie, set_char_set, __set_char_set),
    Init_Nfunc_Entry(4, Trie, insert, __insert),
    Init_Nfunc_Entry(5, Trie, search, __search),
    Init_Nfunc_Entry(6, Trie, search_prefix, __search_prefix),
    Init_Nfunc_Entry(7, Trie, delete, __delete),
    Init_End___Entry(8, Trie),
};
REGISTER_CLASS("Trie", trie_class_info);

