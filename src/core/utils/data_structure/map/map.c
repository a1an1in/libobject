#include <libobject/core/utils/data_structure/map.h>
#include <libobject/attrib_priority.h>

map_module_t map_modules[MAP_TYPE_MAX_NUM];

map_t * map_alloc(allocator_t *allocator, uint8_t type)
{
    map_t *map;
    int ret;

    map            = (map_t *)allocator_mem_alloc(allocator, sizeof(map_t));
    map->map_type  = type;
    map->map_ops   = &map_modules[type].map_ops;
    map->it_ops    = &map_modules[type].it_ops;
    map->allocator = allocator;

    ret = map->map_ops->map_alloc(map);
    if( ret < 0 ) {
        dbg_str(DBG_ERROR, "map_alloc");
        return NULL;
    }

    return map;
}

