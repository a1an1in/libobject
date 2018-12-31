#include <libobject/core/utils/data_structure/map.h>
#include <libobject/core/utils/registry/registry.h>

static int __map_alloc(map_t *map)
{
    map->priv.hash_map =  hash_map_alloc(map->allocator);
    if(map->priv.hash_map == NULL) {
        return -1;
    }

    return 1;
}

static int __map_set(map_t *map, char *attrib, char *value)
{
    return hash_map_set(map->priv.hash_map, attrib, value);
}

static int __map_init(map_t *map)
{
    return  hash_map_init(map->priv.hash_map);
}

static int __map_insert(map_t *map, char *key, void *value)
{
    return hash_map_insert(map->priv.hash_map, key, value);
}

static int __map_search(map_t *map, void *key, map_iterator_t *it)
{
    int ret;

    ret = hash_map_search(map->priv.hash_map, key, &it->pos.hash_map_pos);
    it->map = map;

    return ret;
}

static int __map_del(map_t *map, map_iterator_t *it)
{
    return hash_map_delete(map->priv.hash_map, &it->pos.hash_map_pos);
}

static int __map_destroy(map_t *map)
{
    return hash_map_destroy(map->priv.hash_map);
}

static int __map_begin(map_t *map, map_iterator_t *it)
{
    return hash_map_begin(map->priv.hash_map, &it->pos.hash_map_pos);
}

static int  __map_end(map_t *map, map_iterator_t *it)
{
    return hash_map_end(map->priv.hash_map, &it->pos.hash_map_pos);

}

static int __map_next(map_iterator_t *it, map_iterator_t *next)
{
    return hash_map_pos_next(&it->pos.hash_map_pos, &next->pos.hash_map_pos);

}
static int __map_prev(map_iterator_t *it, map_iterator_t *prev)
{
    return 0;
}
static int __map_equal(map_iterator_t *it1, map_iterator_t *it2)
{
    return hash_map_pos_equal(&it1->pos.hash_map_pos, &it2->pos.hash_map_pos);

}
static void * __map_interator_get_pointer(map_iterator_t *it)
{
    return hash_map_pos_get_pointer(&it->pos.hash_map_pos);

}

int hash_map_pk_register()
{
    map_module_t m = {
        .name     = "hash_map", 
        .map_type = MAP_TYPE_HASH_MAP, 
        .map_ops = {
            .map_alloc       = __map_alloc, 
            .map_set         = __map_set, 
            .map_init        = __map_init, 
            .map_search      = __map_search, 
            .map_insert      = __map_insert, 
            .map_del         = __map_del, 
            .map_destroy     = __map_destroy, 
            .map_begin       = __map_begin, 
            .map_end         = __map_end, 
        }, 
        .it_ops = {
            .map_next        = __map_next, 
            .map_prev        = __map_prev, 
            .map_equal       = __map_equal, 
            .map_get_pointer = __map_interator_get_pointer
        }
    };
    memcpy(&map_modules[MAP_TYPE_HASH_MAP], &m, sizeof(map_module_t));
    return 0;
}
REGISTER_CTOR_FUNC(REGISTRY_CTOR_PRIORITY_REGISTER_MAP_MODULES,
                   hash_map_pk_register);

