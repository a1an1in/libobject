/*  
 * Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 *  
 *  
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
#include <stdio.h>
#include <libobject/attrib_priority.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/alloc/allocator.h>
#include <libobject/core/utils/registry/registry.h>

allocator_module_t allocator_modules[ALLOCATOR_TYPE_LAST];
allocator_t *global_allocator_default;

allocator_t *allocator_create(uint8_t allocator_type,uint8_t lock_type)
{
    allocator_t *p;
    p = (allocator_t *)malloc(sizeof(allocator_t));
    if(p == NULL){
        dbg_str(DBG_ERROR,"allocator_create");
        return p;
    }
    p->allocator_type = allocator_type;
    p->lock_type      = lock_type;
    p->alloc_count    = 0;

    sync_lock_init(&p->head_lock,lock_type);

    return p;
}
//allocate mem_alloc of container i coded
void allocator_ctr_init(allocator_t * alloc,
                        uint32_t slab_array_max_num,
                        uint32_t data_min_size,
                        uint32_t mempool_capacity)
{
    ctr_alloc_t *alloc_p = &alloc->priv.ctr_alloc;

    alloc_p->slab_array_max_num = slab_array_max_num;
    alloc_p->data_min_size      = data_min_size;
    alloc_p->mempool_capacity   = mempool_capacity;
    allocator_modules[alloc->allocator_type].alloc_ops.init(alloc);

}
void allocator_destroy(allocator_t * alloc)
{
    uint8_t allocator_type = alloc->allocator_type;

    dbg_str(ALLOC_DETAIL,"allocator_destroy");

    if(allocator_modules[allocator_type].alloc_ops.destroy){
        allocator_modules[allocator_type].alloc_ops.destroy(alloc);
    }
    free(alloc);
}

allocator_t * allocator_get_default_alloc()
{
    return global_allocator_default;
}

int default_allocator_constructor()
{
    allocator_t *allocator;

    ATTRIB_PRINT("REGISTRY_CTOR_PRIORITY=%d, construct default allocator\n",
                 REGISTRY_CTOR_PRIORITY_DEFAULT_ALLOCATOR);

#if 0
    if((allocator = allocator_create(ALLOCATOR_TYPE_SYS_MALLOC,0) ) == NULL){
        dbg_str(DBG_ERROR,"proxy_create allocator_creator err");
        exit(1);
    }
#else 
    allocator = allocator_create(ALLOCATOR_TYPE_CTR_MALLOC,1);
    allocator_ctr_init(allocator, 0, 8, 0);
#endif
    global_allocator_default = allocator;

    return 0;
}
REGISTER_CTOR_FUNC(REGISTRY_CTOR_PRIORITY_DEFAULT_ALLOCATOR, 
                   default_allocator_constructor);

int default_allocator_destructor()
{
    allocator_t *allocator = allocator_get_default_alloc();

    ATTRIB_PRINT("REGISTRY_DTOR_PRIORITY=%d, "
                 "default allocator destructor, destruct default allocator, alloc count =%d\n",
                 REGISTRY_DTOR_PRIORITY_DEFAULT_ALLOCATOR, 
                 allocator->alloc_count);

    if (allocator->alloc_count > 0) {
        allocator_mem_info(allocator);
        dbg_str(DBG_WARNNING,
                "memory leak, there are %d mem allocs havn't been released! "
                "if you use default allocator, I'll release for you.", 
                allocator->alloc_count);
    }

    allocator_destroy(allocator);

    return 0;
}
REGISTER_DTOR_FUNC(REGISTRY_DTOR_PRIORITY_DEFAULT_ALLOCATOR,
                   default_allocator_destructor);
