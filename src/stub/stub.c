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

int fsh_stub_alloc(stub_t **stub)
{
    stub_t * s;
    int ret;

    TRY {
        THROW_IF(stub == NULL, -1);
        s = stub_alloc();
        THROW_IF(s == NULL, -1);
        *stub = s;
    } CATCH (ret) {}

    return ret;
}

int fsh_stub_free(stub_t *stub)
{
    return stub_free(stub);
}

int fsh_stub_remove_hooks(stub_t *stub)
{
    return stub_remove_hooks(stub);
}