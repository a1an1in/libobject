/**
 * @file map.h
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2016-11-19
 */

/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#ifndef __MAP_H__
#define __MAP_H__

#include <libobject/core/utils/data_structure/map_struct.h>

map_t * map_alloc(allocator_t *allocator,uint8_t type);
int hash_map_pk_register();

static inline int map_set(map_t *map, char *attrib, char *value)
{
    return map->map_ops->map_set(map,attrib,value);
}

static inline int map_init(map_t *map)
{
    return map->map_ops->map_init(map);
}

static inline int map_insert(map_t *map, char *key, void *value)
{
    return map->map_ops->map_insert(map, key, value);
}

static inline int map_search(map_t *map, void *key,map_iterator_t *it)
{
    return map->map_ops->map_search(map,key,it);
}

static inline int map_del(map_t *map, map_iterator_t *it)
{
    return map->map_ops->map_del(map,it);
}

static inline int map_destroy(map_t *map)
{
    map->map_ops->map_destroy(map);
    allocator_mem_free(map->allocator, map);

    return 0;
}

static inline int map_begin(map_t *map, map_iterator_t *it)
{
    map->map_ops->map_begin(map, it);
    it->map = map;

    return 0;
}

static inline int map_end(map_t *map, map_iterator_t *it)
{
    map->map_ops->map_end(map, it);
    it->map = map;

    return 0;
}

static inline int  map_next(map_iterator_t *it, map_iterator_t *next)
{
    it->map->it_ops->map_next(it,next);
    next->map = it->map;

    return 0;
}

static inline int map_prev(map_iterator_t *it, map_iterator_t *prev)
{
    it->map->it_ops->map_prev(it,prev);
    prev->map = it->map;

    return 0;
}

static inline int map_equal(map_iterator_t *it1,map_iterator_t *it2)
{
    return it1->map->it_ops->map_equal(it1,it2);
}

static inline void * map_get_pointer(map_iterator_t *it)
{
    return it->map->it_ops->map_get_pointer(it);
}

static inline void 
map_for_each(map_t *map,void (*func)(map_iterator_t *it))
{
	map_iterator_t it,next,end;

    map_end(map, &end);
	for(	map_begin(map,&it),map_next(&it,&next); 
			!map_equal(&it,&end);
			it = next,map_next(&it,&next)){
		func(&it);
	}
}

#endif
