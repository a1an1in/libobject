#include <libobject/utils/data_structure/map.h>
#include <libobject/attrib_priority.h>

map_module_t map_modules[MAP_TYPE_MAX_NUM];

map_t * map_alloc(allocator_t *allocator,uint8_t type)
{
    map_t *map;
    int ret;

    map            = (map_t *)allocator_mem_alloc(allocator,sizeof(map_t));
    map->map_type  = type;
    map->map_ops   = &map_modules[type].map_ops;
    map->it_ops    = &map_modules[type].it_ops;
    map->allocator = allocator;

    ret = map->map_ops->map_alloc(map);
    if( ret < 0 ) {
        dbg_str(DBG_ERROR,"map_alloc");
        return NULL;
    }

    return map;
}

__attribute__((constructor(ATTRIB_PRIORITY_REGISTER_MAP_MODULES))) void
register_map_modules()
{
    ATTRIB_PRINT("constructor ATTRIB_PRIORITY_REGISTER_MAP_MODULES=%d,register map_modules\n",
                 ATTRIB_PRIORITY_REGISTER_MAP_MODULES);
    hash_map_pk_register();

}

//----------------------------test start---------------------------->>
struct A{
    int a;
    int b;
};

void test_print_context(map_iterator_t *it)
{
    struct A *p = (struct A *)map_get_pointer(it);
    dbg_str(DBG_DETAIL,"a =%d, b=%d",p->a,p->b);
}
void test_map()
{
    map_t *map;
    map_iterator_t it;
    allocator_t *allocator = allocator_get_default_alloc();

    struct A t1 = {1,2};
    struct A t2 = {2,2};
    struct A t3 = {3,2};
    struct A t4 = {4,2};

    map = map_alloc(allocator,MAP_TYPE_HASH_MAP);
    map_init(map, 2, sizeof(struct A));

    dbg_str(DBG_DETAIL,"------------map insert------------");

    map_insert(map,(char *)"11",&t1);
    map_insert(map,(char *)"22",&t2);
    map_insert(map,(char *)"55",&t4);
    map_insert(map,(char *)"33",&t3);

    dbg_str(DBG_DETAIL,"------------map search------------");
    struct A *p;
    map_search(map,(char *)"33",&it);

    p = (struct A *)map_get_pointer(&it);
    dbg_str(DBG_DETAIL,"a =%d, b=%d",p->a,p->b);

    dbg_str(DBG_DETAIL,"------------map for each------------");
    map_for_each(map,test_print_context);

    dbg_str(DBG_DETAIL,"------------map destroy------------");
    map_destroy(map);

}

//----------------------------test end---------------------------<<
