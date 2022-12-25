#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/concurrent/Producer.h>
#include <libobject/version.h>

#if (defined(ANDROID_USER_MODE))
#include <android/log.h>
#elif (defined(WINDOWS_USER_MODE))
#include <winsock2.h>
#endif


int libobject_init_core()
{
#if (defined(WINDOWS_USER_MODE))
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 1), &wsa_data)) {
        dbg_str(NET_ERROR, "WSAStartup error");
        return -1;
    }
#endif

    execute_ctor_funcs();

    libobject_init_fs();

    return 1;
}

int libobject_destroy_core()
{
//#if (defined(WINDOWS_USER_MODE))
//    WSACleanup();
//#endif
    libobject_destroy_fs();
    execute_dtor_funcs();

    return 1;
}
