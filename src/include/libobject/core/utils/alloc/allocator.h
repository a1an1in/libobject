#ifndef __ALLOCATOR_H__
#define __ALLOCATOR_H__

#include <libobject/core/utils/sync_lock.h>
#include <libobject/core/utils/alloc/allocator_ctr_alloc.h>
#include <libobject/core/utils/backtrace.h>

enum allocator_type{
    ALLOCATOR_TYPE_SYS_MALLOC = 0,
    ALLOCATOR_TYPE_CTR_MALLOC,
    ALLOCATOR_TYPE_LAST
};

typedef struct allocator{
#	define COTAINOR_NAME_MAX_LEN 40
    char name[COTAINOR_NAME_MAX_LEN];
    uint8_t allocator_type;
    uint8_t lock_type;
    union{
        ctr_alloc_t ctr_alloc;
    }priv;
    struct allocator_operations *alloc_ops;
    uint32_t alloc_count;
    sync_lock_t head_lock;    
#	undef COTAINOR_NAME_MAX_LEN
} allocator_t;

struct allocator_operations{
    int (*init)(allocator_t *alloc);
    void *(*alloc)(allocator_t *alloc,uint32_t size);
    void (*free)(allocator_t *alloc,void *addr);
    void (*destroy)(allocator_t * alloc);
    void (*info)(allocator_t * alloc);
    void (*tag)(allocator_t *alloc,void *addr, void *tag);
};

typedef struct allocator_module{
    uint8_t allocator_type;
    struct allocator_operations alloc_ops;
} allocator_module_t;

extern allocator_module_t allocator_modules[ALLOCATOR_TYPE_LAST];

allocator_t *allocator_create(uint8_t allocator_type,uint8_t lock_type);
void allocator_destroy(allocator_t * alloc);
void allocator_ctr_init(allocator_t * alloc,
                        uint32_t slab_array_max_num,
                        uint32_t data_min_size,
                        uint32_t mempool_capacity);
static inline void allocator_mem_tag(allocator_t * alloc,void *addr, void *tag);
allocator_t * allocator_get_default_instance();
int allocator_save_upper_nlayer_name(allocator_t *allocator, int n, void *dst);

#if 0

static inline void *
allocator_mem_alloc(allocator_t * alloc,uint32_t size)
{
    void *ret = NULL;

    sync_lock(&alloc->head_lock,NULL);
    ret = allocator_modules[alloc->allocator_type].alloc_ops.alloc(alloc,size);
    sync_unlock(&alloc->head_lock);

    return ret;
}

#else

static inline void *
__allocator_mem_alloc(allocator_t * alloc,uint32_t size)
{
    void *ret = NULL;

    if (size == 0) {
        return NULL;
    }
    sync_lock(&alloc->head_lock,NULL);
    ret = allocator_modules[alloc->allocator_type].alloc_ops.alloc(alloc,size);
    sync_unlock(&alloc->head_lock);

    return ret;
}

#define allocator_mem_alloc(alloc, size) ({\
    void *ret = NULL;\
    ret =  __allocator_mem_alloc(alloc, size);\
    if (ret != NULL) {\
        char tmp[1024];\
        sprintf(tmp, "%d:%s", __LINE__, extract_filename_in_macro(__FILE__));\
        allocator_save_upper_nlayer_name(alloc, 2, ret);\
    }\
    ret;\
})

#define allocator_mem_zalloc(alloc, size) ({\
    void *ret = NULL;\
    ret =  __allocator_mem_alloc(alloc, size);\
    if (ret != NULL) {\
        char tmp[1024];\
        memset(ret, 0, size);\
        sprintf(tmp, "%d:%s", __LINE__, extract_filename_in_macro(__FILE__));\
        allocator_mem_tag(alloc,ret, tmp);\
    }\
    ret;\
})

#endif

static inline int 
allocator_mem_free(allocator_t * alloc,void *addr)
{
    if (addr == NULL) return 0;
    sync_lock(&alloc->head_lock,NULL);
    allocator_modules[alloc->allocator_type].alloc_ops.free(alloc,addr);
    sync_unlock(&alloc->head_lock);
}

static inline void 
allocator_mem_tag(allocator_t * alloc,void *addr, void *tag)
{
    sync_lock(&alloc->head_lock,NULL);
    if(allocator_modules[alloc->allocator_type].alloc_ops.tag) {
        allocator_modules[alloc->allocator_type].alloc_ops.tag(alloc,addr, tag);
    }
    sync_unlock(&alloc->head_lock);
}

static inline void 
allocator_mem_info(allocator_t * alloc)
{
    if(allocator_modules[alloc->allocator_type].alloc_ops.info)
        allocator_modules[alloc->allocator_type].alloc_ops.info(alloc);
}

#endif
