#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#include <libobject/user_mode.h>
#include <libobject/basic_types.h>

enum filesystem_type_e {
	FILESYSTEM_TYPE_UNDEFINED = 0,
	FILESYSTEM_TYPE_UNIX,
	FILESYSTEM_TYPE_WINDOS,
	FILESYSTEM_TYPE_MAX_NUM,
};

typedef struct filesystem_s{
	union fs_u{
	} fs;
    struct filesystem_operations  *ops;
} filesystem_t;

struct filesystem_operations {
	int (*is_directory)(filesystem_t *fs, char *name);
	int (*list)(filesystem_t *fs, char *name, char **list, int count);
};

typedef struct filesystem_module_s {
	char name[20];
	uint8_t filesystem_type;
	struct filesystem_operations ops;
} filesystem_module_t;

extern filesystem_module_t filesystem_modules[FILESYSTEM_TYPE_MAX_NUM];
static inline int fs_is_directory(filesystem_t *fs, char *name);

#endif
