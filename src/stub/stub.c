/**
 * @file Stub.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-04-18
 */

#include "libobject/stub/stub.h"
#include "libobject/stub/admin.h"

stub_t *stub_alloc()
{
    return stub_admin_alloc(stub_admin_get_default_instance());
}

int stub_free(stub_t *stub)
{
    if (stub == NULL) return -1;

    return stub_admin_free(stub_admin_get_default_instance(), stub);
}

int stub_remove_hooks(stub_t *stub)
{
    if (stub == NULL) return -1;

    return stub_remove(stub);
}