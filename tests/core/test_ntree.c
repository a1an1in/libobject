/**
 * @file test_ntree.c
 * @Synopsis
 *     通用 N 叉树容器 NTree 的单测:
 *       - 多叉插入 + 子节点按 name 升序 + 同父同名冲突拒绝
 *       - node_find / count / node_depth
 *       - 递归前序(preorder)顺序
 *       - node_detach 摘除
 *       - node_free / clear(含 free_data 回调释放用户载荷)
 *     路径拆分(get-or-create)属于应用层职责, 这里用一个示例函数演示.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/NTree.h>
#include <libobject/mockery/mockery.h>

/* 应用层示例: 把 "a/b/c" 按 '/' 拆段, 中间目录 get-or-create, 末端为节点
 * (末端 data 不为 NULL). 返回末端节点; 末端与已有同名节点冲突返回 NULL. */
static ntree_node_t *__insert_path(NTree *tree, const char *path, void *data)
{
    char tmp[512];
    char *segs[32];
    int nseg = 0;
    ntree_node_t *cur = tree->get_root(tree);
    ntree_node_t *n = NULL;
    char *p, *save;
    int i;

    strncpy(tmp, path, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    for (p = strtok_r(tmp, "/", &save); p && nseg < 32; p = strtok_r(NULL, "/", &save))
        segs[nseg++] = p;
    if (nseg == 0) return NULL;

    for (i = 0; i < nseg; i++) {
        int is_last = (i == nseg - 1);
        n = tree->node_find(tree, cur, segs[i]);
        if (n == NULL) {
            n = tree->node_new(tree, segs[i], is_last ? data : NULL);
            if (n == NULL) return NULL;
            if (tree->node_insert(tree, cur, n) != 0) {
                tree->node_free(tree, n);
                return NULL;                 /* 同父同名冲突 */
            }
        } else if (is_last) {
            return NULL;                     /* 末端同名已存在 */
        }
        cur = n;
    }
    return n;
}

/* 校验子节点按 name 升序 */
static int __children_sorted(ntree_node_t *d)
{
    int i;
    for (i = 1; i < d->nchild; i++) {
        if (d->child[i - 1]->name == NULL || d->child[i]->name == NULL) return 0;
        if (strcmp(d->child[i - 1]->name, d->child[i]->name) > 0) return 0;
    }
    return 1;
}

static int test_ntree_insert_order_and_count(TEST_ENTRY *entry)
{
    NTree *tree;
    allocator_t *allocator = allocator_get_default_instance();
    ntree_node_t *root, *b, *a;
    int ret = 0;

    TRY {
        tree = object_new(allocator, "NTree", NULL);
        THROW_IF(tree == NULL, -1);

        root = tree->node_new(tree, "root", NULL);
        THROW_IF(root == NULL, -1);
        tree->set_root(tree, root);
        THROW_IF(tree->count(tree) != 1, -1);

        /* 乱序插入, 期望子节点最终按名升序 */
        b = tree->node_new(tree, "b", NULL);
        a = tree->node_new(tree, "a", NULL);
        THROW_IF(tree->node_insert(tree, root, b) != 0, -1);
        THROW_IF(tree->node_insert(tree, root, a) != 0, -1);
        THROW_IF(tree->node_insert(tree, root, tree->node_new(tree, "c", NULL)) != 0, -1);

        THROW_IF(root->nchild != 3, -1);
        THROW_IF(__children_sorted(root) != 1, -1);
        THROW_IF(tree->count(tree) != 4, -1);
        THROW_IF(tree->node_depth(tree, a) != 2, -1);

        /* 同父同名插入应被拒绝 */
        THROW_IF(tree->node_insert(tree, root, tree->node_new(tree, "a", NULL)) != -1, -1);

        /* find */
        THROW_IF(tree->node_find(tree, root, "b") == NULL, -1);
        THROW_IF(tree->node_find(tree, root, "z") != NULL, -1);

        /* 摘除 b */
        THROW_IF(tree->node_detach(tree, b) != 0, -1);
        THROW_IF(tree->node_count != 3, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(tree);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_ntree_insert_order_and_count);

/* 前序(先父后子、子按序)遍历顺序 */
typedef struct __visit_ctx_s {
    char out[512];
    int len;
} __visit_ctx_t;

static int __collect(ntree_node_t *n, void *ctx)
{
    __visit_ctx_t *c = ctx;
    if (n->name) {
        c->len += snprintf(c->out + c->len, sizeof(c->out) - c->len, "%s;", n->name);
    }
    return 0;
}

static int test_ntree_preorder(TEST_ENTRY *entry)
{
    NTree *tree;
    allocator_t *allocator = allocator_get_default_instance();
    ntree_node_t *root;
    __visit_ctx_t c = { {0}, 0 };
    int ret = 0;

    TRY {
        tree = object_new(allocator, "NTree", NULL);
        THROW_IF(tree == NULL, -1);

        /* 目录树: root -> { a -> { a1, a2 }, b } */
        root = tree->node_new(tree, "root", NULL);
        tree->set_root(tree, root);
        THROW_IF(__insert_path(tree, "a", (void *)1) == NULL, -1);
        THROW_IF(__insert_path(tree, "a/a1", (void *)2) == NULL, -1);
        THROW_IF(__insert_path(tree, "a/a2", (void *)3) == NULL, -1);
        THROW_IF(__insert_path(tree, "b", (void *)4) == NULL, -1);
        THROW_IF(tree->count(tree) != 5, -1);

        tree->preorder(tree, NULL, __collect, &c);
        /* 子节点均按名升序: 期望 root;a;a1;a2;b; */
        THROW_IF(strcmp(c.out, "root;a;a1;a2;b;") != 0, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(tree);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_ntree_preorder);

/* 路径冲突: 同一父下已存在同名(文件)时, 再插入同名叶子应失败 */
static int test_ntree_path_conflict(TEST_ENTRY *entry)
{
    NTree *tree;
    allocator_t *allocator = allocator_get_default_instance();
    ntree_node_t *root;
    int ret = 0;

    TRY {
        tree = object_new(allocator, "NTree", NULL);
        THROW_IF(tree == NULL, -1);

        root = tree->node_new(tree, "root", NULL);
        tree->set_root(tree, root);
        THROW_IF(__insert_path(tree, "test.txt", (void *)1) == NULL, -1);
        /* 另一同名文件落在不同目录下, 应互不冲突 */
        THROW_IF(__insert_path(tree, "sub/test.txt", (void *)2) == NULL, -1);
        /* root + test.txt + sub + sub/test.txt = 4 */
        THROW_IF(tree->count(tree) != 4, -1);
        /* 同目录再插同名叶子应失败 */
        THROW_IF(__insert_path(tree, "test.txt", (void *)9) != NULL, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(tree);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_ntree_path_conflict);

/* clear 时按 free_data 回调释放用户载荷 */
static int g_data_freed = 0;

static void __free_data(void *data)
{
    if (data) {
        g_data_freed++;
        free(data);
    }
}

static int test_ntree_clear_releases_data(TEST_ENTRY *entry)
{
    NTree *tree;
    allocator_t *allocator = allocator_get_default_instance();
    ntree_node_t *root, *n;
    int *payload;
    int ret = 0;

    TRY {
        tree = object_new(allocator, "NTree", NULL);
        THROW_IF(tree == NULL, -1);

        g_data_freed = 0;
        tree->free_data = __free_data;

        root = tree->node_new(tree, NULL, NULL);
        tree->set_root(tree, root);

        n = tree->node_new(tree, "f", NULL);
        payload = (int *)calloc(1, sizeof(int));
        THROW_IF(payload == NULL, -1);
        /* node_new 之后再单独把 data 挂上(演示 data 可后填) */
        n->data = payload;
        THROW_IF(tree->node_insert(tree, root, n) != 0, -1);

        THROW_IF(__insert_path(tree, "d/f", NULL) == NULL, -1);

        THROW_IF(tree->count(tree) != 4, -1);   /* root+f+d+f */
        tree->clear(tree);
        THROW_IF(tree->count(tree) != 0, -1);
        THROW_IF(g_data_freed != 1, -1);        /* 仅 "f" 节点挂了 data */
    } CATCH (ret) {} FINALLY {
        object_destroy(tree);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_ntree_clear_releases_data);
