#ifndef __MAP_STRUCT_H__
#define __MAP_STRUCT_H__

#include <libobject/basic_types.h>
#include <libobject/core/utils/data_structure/hash_list.h>
#include <libobject/core/utils/data_structure/rbtree_map.h>

enum map_type{
    MAP_TYPE_RBTREE_MAP,
    MAP_TYPE_HASH_MAP,
    MAP_TYPE_MAX_NUM
};

typedef struct map_interator{
    union{
        rbtree_map_pos_t rbtree_map_pos;
        hash_map_pos_t hash_map_pos;
    }pos;
    struct map_s *map;
}map_iterator_t;


typedef struct map_s{
#	define MAP_NAME_MAX_LEN 40
    char name[MAP_NAME_MAX_LEN];
    uint8_t map_type;
    allocator_t *allocator;
    struct map_operations *map_ops;
    struct map_interator_operations *it_ops;
    struct map_interator begin,end,cur;
    pthread_rwlock_t head_lock;    
    union{
        rbtree_map_t *rbtree_map;
        hash_map_t *hash_map;
    }priv;
#	undef MAP_NAME_MAX_LEN
}map_t;

struct map_operations{
    int (*map_alloc)(map_t *map);
    int (*map_set)(map_t *map, char *attrib, char *value);
    int (*map_init)(map_t *map);
    int (*map_insert)(map_t *map, char *key, void *value);
    int (*map_search)(map_t *map, void *key,map_iterator_t *it);
    int (*map_del)(map_t *map, map_iterator_t *it);
    int (*map_destroy)(map_t *map);
    int (*map_begin)(map_t *map,map_iterator_t *it);
    int (*map_end)(map_t *map,map_iterator_t *it);
};

struct map_interator_operations{
    int (*map_next)(map_iterator_t *it, map_iterator_t *next);
    int (*map_prev)(map_iterator_t *it, map_iterator_t *next);
    int (*map_equal)(map_iterator_t *it1,map_iterator_t *it2);
    void *(*map_get_pointer)(map_iterator_t *it);
};

typedef struct map_module{
#	define MAP_NAME_MAX_LEN 20
    char name[MAP_NAME_MAX_LEN];
    uint8_t map_type;
    struct map_operations map_ops;
    struct map_interator_operations it_ops;
    struct map_interator begin,end;
#	undef MAP_NAME_MAX_LEN
}map_module_t;

extern map_module_t map_modules[MAP_TYPE_MAX_NUM];

#endif
