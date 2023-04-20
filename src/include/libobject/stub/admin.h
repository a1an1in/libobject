#ifndef __STUB_ADMIN_H__
#define __STUB_ADMIN_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/List.h>
#include <libobject/stub/stub.h>>

typedef struct placeholder_s placeholder_t;

typedef struct stub_admin_s {
    List *placeholder_list;
    List *free_stub_list;
    placeholder_t *cur;
} stub_admin_t;

struct placeholder_s {
    void *addr;
    int size;
    int index;
};

int stub_admin_init_default_instance();
int stub_admin_destroy_default_instance();
stub_admin_t *stub_admin_get_default_instance();
stub_t *stub_admin_alloc(stub_admin_t *admin);
int stub_admin_free(stub_admin_t *admin, stub_t *stub);

#endif
