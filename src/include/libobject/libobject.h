#ifndef __LIBOBJECT_H__
#define __LIBOBJECT_H__

#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>

/*
 *typedef struct libobject_s libobject_t;
 *
 *#define MAX_LIBOBJECT_PATH_LEN 50
 *struct libobject_s {
 *    char path[MAX_LIBOBJECT_PATH_LEN];
 *};
 *
 *int libobject_set(libobject_t *libobject, char *attrib, void *value);
 *int libobject_init(libobject_t *libobject);
 */

int libobject_set_run_path(char *path);
char *libobject_get_run_path();
int libobject_init();
int libobject_exit();

#endif
