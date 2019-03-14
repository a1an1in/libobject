#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/libobject.h>
#include <libobject/concurrent/producer.h>
#include <libobject/version.h>

#if (defined(ANDROID_USER_MODE))
#include <android/log.h>
#endif

#define MAX_LIBOBJECT_PATH_LEN 50
#define DEFAULT_LIBOBJECT_RUN_PATH "/tmp"
char libobject_path[MAX_LIBOBJECT_PATH_LEN];
#undef MAX_LIBOBJECT_PATH_LEN


int libobject_set_run_path(char *path)
{
    strcpy(libobject_path, path);

    return 0;
}

char *libobject_get_run_path()
{
    if (strlen(libobject_path) == 0) {
        strcpy(libobject_path, DEFAULT_LIBOBJECT_RUN_PATH);
    }

    return libobject_path;
}

int libobject_init()
{
    execute_ctor_funcs();

    return 0;
}

int libobject_exit()
{
    kill(0, SIGQUIT);

    return 0;
}

static int print_library_version()
{
#if (defined(ANDROID_USER_MODE))
    __android_log_print(ANDROID_LOG_INFO, "libobject",
            "libobject version:%d.%d.%d.%d\n", 
            LIBOBJECT_VERSION_MAJOR, LIBOBJECT_VERSION_MINOR,
            LIBOBJECT_VERSION_MAINTENANCE, LIBOBJECT_VERSION_BUILD);
#else
    printf("libobject version:%d.%d.%d.%d\n", 
            LIBOBJECT_VERSION_MAJOR, LIBOBJECT_VERSION_MINOR,
            LIBOBJECT_VERSION_MAINTENANCE, LIBOBJECT_VERSION_BUILD);
#endif

    return 0;
}
REGISTER_CTOR_FUNC(REGISTRY_CTOR_PRIORITY_VERSION, print_library_version);

int __attribute__((destructor)) destruct_libobject()
{
    execute_dtor_funcs();

    return 0;
}
