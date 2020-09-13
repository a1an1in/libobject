#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/libobject.h>
#include <libobject/concurrent/Producer.h>
#include <libobject/version.h>

#if (defined(ANDROID_USER_MODE))
#include <android/log.h>
#elif (defined(WINDOWS_USER_MODE))
#include <winsock2.h>
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
#if (defined(WINDOWS_USER_MODE))
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 1), &wsa_data)) {
        dbg_str(NET_ERROR, "WSAStartup error");
        return -1;
    }
#endif
    execute_ctor_funcs();

    return 1;
}

int libobject_exit()
{
#if (defined(WINDOWS_USER_MODE))
    WSACleanup();
#endif
#if (!defined(WINDOWS_USER_MODE))
    kill(0, SIGQUIT);
#endif
    return 1;
}

int __attribute__((destructor)) destruct_libobject()
{
    execute_dtor_funcs();

    return 1;
}
