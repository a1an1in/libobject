#include <libobject/core/utils/filesystem.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>


static int __list(filesystem_t *fs, char *name, char **list, int count)
{
}

static int __is_directory(filesystem_t *fs, char *name)
{
}

static int  register_filesystem_moudle() {
    filesystem_module_t fs_module = {
        .name = "unix_filesystem", 
        .ops = {
            .list    = __list, 
            .is_directory = __is_directory,
        }, 
    };
    ATTRIB_PRINT("REGISTRY_CTOR_PRIORITY=%d, register unix filesystem module\n", 
                 REGISTRY_CTOR_PRIORITY_FILESYSTEM_MODULES);
    memcpy(&filesystem_modules[FILESYSTEM_TYPE_UNIX], &fs_module, sizeof(filesystem_module_t));

    return 0;
}
REGISTER_CTOR_FUNC(REGISTRY_CTOR_PRIORITY_FILESYSTEM_MODULES, 
                   register_filesystem_moudle);
