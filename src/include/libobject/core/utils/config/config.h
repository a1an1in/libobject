#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <libobject/core/utils/alloc/allocator.h>

#define DEFAULT_CONFIG_BUF_LEN 1024
typedef struct configurator_s{
    allocator_t *allocator;
    void *buf;
    int buf_len;
}configurator_t;

configurator_t *cfg_alloc(allocator_t *allocator);
int cfg_config(configurator_t * c, const char *path, int type, const char *name, void *value);
int cfg_config_str(configurator_t * c, const char *path, const char *name, void *value); 
int cfg_config_num(configurator_t * c, const char *path, const char *name, int value);
int cfg_destroy(configurator_t * c);
#endif

