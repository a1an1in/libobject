#ifndef __NTREE_H__
#define __NTREE_H__

#include <libobject/core/Obj.h>

/**
 * 通用 N 叉(多叉)树容器.
 *
 * 与库里二叉类容器(RBTree_Map / Interval_Tree)或字符级前缀树(Trie)不同,
 * 每个节点可有任意多个子节点, 是"通用多叉树". 典型用途是表达目录/路径层级
 * (如 squashfs 写端把 add 进来的相对路径 sub/test.txt 按 '/' 拆段建目录树).
 *
 * 约定:
 *  - 子节点默认按 name 升序存放; 同一父节点下不允许同名(NULL 名字节点追加在尾部).
 *  - 节点 name 由容器 strdup 并负责释放; data 为不透明用户载荷, 容器不理解其语义,
 *    释放策略由 NTree.free_data 回调决定(NULL 则 data 交由调用方管理).
 *  - data 的构造也可注入: 设置 alloc_data 后, node_new 建好节点即回调
 *    alloc_data(node, arg), 把返回值写入 node->data(返回 NULL 视为失败);
 *    未设置 alloc_data 时, node_new 的 arg 直接作为 data.
 *  - 所有节点内存都由容器(经 parent.allocator)分配/释放.
 *  - preorder 用递归前序实现, 遍历以 root 为根的整棵树并对每个节点调用调用方
 *    注入的访问器(visit(node, ctx)); 树高不宜过大(目录/路径层级场景天然低).
 */
typedef struct NTree_s NTree;

typedef struct ntree_node_s {
    char *name;                 /* 键名(可选, 可为 NULL), 容器 strdup/释放 */
    void *data;                 /* 用户载荷(不透明) */
    int nchild, cap;
    struct ntree_node_s **child;    /* 子节点数组, 按 name 升序 */
    struct ntree_node_s *parent;    /* 父节点(根为 NULL) */
} ntree_node_t;

struct NTree_s {
    Obj parent;

    int (*construct)(NTree *, char *);
    int (*deconstruct)(NTree *);

    /* 通用操作(全部以容器为上下文, 内部使用 parent.allocator) */
    int             (*set_root)(NTree *tree, ntree_node_t *root);   /* 设/换根, 返回 0 */
    ntree_node_t   *(*get_root)(NTree *tree);
    ntree_node_t   *(*node_new)(NTree *tree, const char *name, void *arg); /* 建游离节点(见 alloc_data) */
    int             (*node_insert)(NTree *tree, ntree_node_t *parent, ntree_node_t *child); /* 同名返回 -1 */
    ntree_node_t   *(*node_find)(NTree *tree, ntree_node_t *parent, const char *name);
    int             (*node_detach)(NTree *tree, ntree_node_t *node); /* 从父摘除(不释放) */
    int             (*node_free)(NTree *tree, ntree_node_t *node);   /* 递归释放子树(根也适用) */
    int             (*clear)(NTree *tree);                           /* 释放整棵树, root=NULL */
    int             (*count)(NTree *tree);                           /* 树节点总数(含 root) */
    int             (*node_depth)(NTree *tree, ntree_node_t *node);  /* 深度(根=1) */
    int             (*preorder)(NTree *tree, ntree_node_t *node,     /* 前序(先父后子)遍历, 回调返回非 0 即停止 */
                                int (*visit)(ntree_node_t *n, void *ctx), void *ctx);

    /* ---- 数据字段 ---- */
    void (*free_data)(void *data);   /* 可选: 释放节点时顺带释放 node->data */
    void *(*alloc_data)(ntree_node_t *node, void *arg); /* 可选: node_new 建节点后回调, 返回值写入 node->data */
    ntree_node_t *root;
    int node_count;
};

#endif
