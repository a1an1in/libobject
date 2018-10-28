#include <stdio.h>
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>

int print_library_version()
{
    ATTRIB_PRINT("REGISTRY_CTOR_PRIORITY=%d, register version info:%s\n", 
                 REGISTRY_CTOR_PRIORITY_VERSION, 
                 LIBRARY_VERSION);

    return 0;
}
REGISTER_CTOR_FUNC(REGISTRY_CTOR_PRIORITY_VERSION, print_library_version);

int __attribute__((destructor)) destruct_libobject()
{
    execute_dtor_funcs();

    return 0;
}
