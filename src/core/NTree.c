/**
 * @file NTree.c
 * @Synopsis
 *     通用 N 叉(多叉)树容器实现.
 *     每个节点(name + data 用户载荷)可有任意多个子节点, 子节点按 name 升序存放,
 *     同一父节点下不允许同名. 典型用途是目录/路径层级: 把一个相对路径按 '/' 拆成
 *     若干段, 逐段 get-or-create 即可得到一颗目录树(如 squashfs 写端).
 *
 *     与 Trie(字符级前缀树)、RBTree_Map/Interval_Tree(二叉)区分:
 *        - 不限定子节点个数(多叉);
 *        - 不绑定任何路径/比较语义, data 是不透明载荷;
 *        - 有序仅是"子节点按 name 升序", 由容器维护.
 *
 *     所有权约定:
 *        - node_new 创建"游离"节点, 不计入 node_count;
 *        - node_insert(挂到某父下)/set_root(作为根)后才计入 node_count;
 *        - node_free/clear 递归释放节点内存(含 name), 若设置了 free_data 则顺带释放 data;
 *        - 全部内存走 tree->parent.allocator.
 * @author Zoo
 * @version 1.0
 * @date 2026-09-02
 */
#include <string.h>
#include <stdlib.h>
#include <libobject/core/NTree.h>
#include <libobject/core/utils/dbg/debug.h>

/* ================================================================== */
/* 节点内部工具                                                        */
/* ================================================================== */

/* 以 n 为根的子树节点数 */
static int __size(ntree_node_t *n)
{
    int s = 1, i;
    for (i = 0; i < n->nchild; i++) s += __size(n->child[i]);
    return s;
}


static int __ensure_cap(NTree *tree, ntree_node_t *d, int need)
{
    allocator_t *allocator = tree->parent.allocator;
    ntree_node_t **nc;
    int ncap;

    if (d->cap >= need) return 0;
    ncap = d->cap ? d->cap : 4;
    while (ncap < need) ncap *= 2;
    nc = allocator_mem_alloc(allocator, (size_t)ncap * sizeof(void *));
    if (nc == NULL) return -1;
    if (d->child) {
        memcpy(nc, d->child, (size_t)d->nchild * sizeof(void *));
        allocator_mem_free(allocator, d->child);
    }
    d->child = nc;
    d->cap = ncap;
    return 0;
}

/* 在 d 的子表中找 name 的插入位置; 已有同名则 *found=1 并返回 -1;
 * name 为 NULL 的节点一律追加到尾部(不排序). */
static int __insert_pos(NTree *tree, ntree_node_t *d, const char *name, int *found)
{
    int i, cmp;
    (void)tree;

    *found = 0;
    if (name == NULL) return d->nchild;
    for (i = 0; i < d->nchild; i++) {
        if (d->child[i]->name == NULL) continue;
        cmp = strcmp(name, d->child[i]->name);
        if (cmp == 0) { *found = 1; return -1; }
        if (cmp < 0) return i;
    }
    return d->nchild;
}

static void __free_rec(NTree *tree, ntree_node_t *n)
{
    allocator_t *allocator = tree->parent.allocator;
    int i;

    for (i = 0; i < n->nchild; i++) __free_rec(tree, n->child[i]);
    if (n->child) allocator_mem_free(allocator, n->child);
    if (n->name) allocator_mem_free(allocator, n->name);
    if (tree->free_data && n->data) tree->free_data(n->data);
    allocator_mem_free(allocator, n);
}

/* ================================================================== */
/* 构造 / 析构                                                        */
/* ================================================================== */

static int __construct(NTree *tree, char *init_str)
{
    tree->root = NULL;
    tree->node_count = 0;
    tree->free_data = NULL;
    tree->alloc_data = NULL;
    return 0;
}

static int __deconstruct(NTree *tree)
{
    if (tree->root) __free_rec(tree, tree->root);
    tree->root = NULL;
    tree->node_count = 0;
    return 0;
}

/* ================================================================== */
/* 公开操作                                                            */
/* ================================================================== */

static int __set_root(NTree *tree, ntree_node_t *root)
{
    if (tree->root) __free_rec(tree, tree->root);
    tree->root = root;
    tree->node_count = 0;
    if (root) {
        root->parent = NULL;
        tree->node_count = __size(root);
    }
    return 0;
}

static ntree_node_t *__get_root(NTree *tree)
{
    return tree->root;
}

static ntree_node_t *__node_new(NTree *tree, const char *name, void *arg)
{
    allocator_t *allocator = tree->parent.allocator;
    ntree_node_t *n = allocator_mem_zalloc(allocator, sizeof(*n));
    if (n == NULL) return NULL;
    if (name != NULL) {
        n->name = allocator_mem_zalloc(allocator, strlen(name) + 1);
        if (n->name == NULL) {
            allocator_mem_free(allocator, n);
            return NULL;
        }
        strcpy(n->name, name);
    }
    n->parent = NULL;
    /* 注入 alloc_data: 建好节点后回调它来分配 data; 未设置则 arg 直接当 data */
    if (tree->alloc_data != NULL) {
        n->data = tree->alloc_data(n, arg);
        if (n->data == NULL) {
            if (n->name) allocator_mem_free(allocator, n->name);
            allocator_mem_free(allocator, n);
            return NULL;
        }
    } else {
        n->data = arg;
    }
    return n;
}

static int __node_insert(NTree *tree, ntree_node_t *parent, ntree_node_t *child)
{
    int pos, found = 0, i;

    if (parent == NULL || child == NULL) return -1;
    if (child->parent != NULL) return -1;              /* 已挂接的节点不能重复插入 */
    if (child == tree->root) return -1;
    pos = __insert_pos(tree, parent, child->name, &found);
    if (found) return -1;                              /* 同父同名冲突, 不接管 child */
    if (__ensure_cap(tree, parent, parent->nchild + 1) != 0) return -1;
    for (i = parent->nchild; i > pos; i--) parent->child[i] = parent->child[i - 1];
    parent->child[pos] = child;
    parent->nchild++;
    child->parent = parent;
    tree->node_count += __size(child);
    return 0;
}

static ntree_node_t *__node_find(NTree *tree, ntree_node_t *parent, const char *name)
{
    int i;
    if (parent == NULL) return NULL;
    if (name == NULL) return NULL;
    for (i = 0; i < parent->nchild; i++) {
        if (parent->child[i]->name && strcmp(parent->child[i]->name, name) == 0)
            return parent->child[i];
    }
    return NULL;
}

static int __node_detach(NTree *tree, ntree_node_t *node)
{
    ntree_node_t *d;
    int i;

    if (node == NULL || node->parent == NULL) return -1;
    d = node->parent;
    for (i = 0; i < d->nchild; i++) {
        if (d->child[i] == node) break;
    }
    if (i >= d->nchild) return -1;
    memmove(&d->child[i], &d->child[i + 1],
            (size_t)(d->nchild - i - 1) * sizeof(void *));
    d->nchild--;
    tree->node_count -= __size(node);
    node->parent = NULL;
    return 0;
}

static int __node_free(NTree *tree, ntree_node_t *node)
{
    if (node == NULL) return -1;
    if (node->parent == NULL) {
        /* 根或游离节点: 直接释放其整棵子树 */
        if (node == tree->root) tree->node_count = 0;
        __free_rec(tree, node);
        if (node == tree->root) tree->root = NULL;
    } else {
        __node_detach(tree, node);
        __free_rec(tree, node);
    }
    return 0;
}

static int __clear(NTree *tree)
{
    if (tree->root) {
        __free_rec(tree, tree->root);
        tree->root = NULL;
        tree->node_count = 0;
    }
    return 0;
}

static int __count(NTree *tree)
{
    return tree->node_count;
}

static int __node_depth(NTree *tree, ntree_node_t *node)
{
    int d = 0;
    if (node == NULL) node = tree->root;
    while (node) {
        d++;
        node = node->parent;
    }
    return d;
}

/* 递归前序遍历: 先访问节点, 再按序访问子节点; 回调返回非 0 立即停止并上抛 */
static int __preorder_impl(NTree *tree, ntree_node_t *node,
                           int (*visit)(ntree_node_t *, void *), void *ctx)
{
    int i, r;

    if (node == NULL) return 0;
    if (visit) {
        r = visit(node, ctx);
        if (r) return r;
    }
    for (i = 0; i < node->nchild; i++) {
        r = __preorder_impl(tree, node->child[i], visit, ctx);
        if (r) return r;
    }
    return 0;
}

static int __preorder(NTree *tree, ntree_node_t *node,
                      int (*visit)(ntree_node_t *, void *), void *ctx)
{
    return __preorder_impl(tree, node ? node : tree->root, visit, ctx);
}

static class_info_entry_t ntree_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, NTree, construct, __construct),
    Init_Nfunc_Entry(2, NTree, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, NTree, set_root, __set_root),
    Init_Nfunc_Entry(4, NTree, get_root, __get_root),
    Init_Nfunc_Entry(5, NTree, node_new, __node_new),
    Init_Nfunc_Entry(6, NTree, node_insert, __node_insert),
    Init_Nfunc_Entry(7, NTree, node_find, __node_find),
    Init_Nfunc_Entry(8, NTree, node_detach, __node_detach),
    Init_Nfunc_Entry(9, NTree, node_free, __node_free),
    Init_Nfunc_Entry(10, NTree, clear, __clear),
    Init_Nfunc_Entry(11, NTree, count, __count),
    Init_Nfunc_Entry(12, NTree, node_depth, __node_depth),
    Init_Nfunc_Entry(13, NTree, preorder, __preorder),
    Init_End___Entry(14, NTree),
};
REGISTER_CLASS(NTree, ntree_class_info);
