#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/net/worker/api.h>

static int test_work_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;
    dbg_str(DBG_DETAIL,"%s", t->buf);
}

void log_server()
{
    allocator_t *allocator = allocator_get_default_instance();
    Client *c = NULL;

    dbg_str(DBG_DETAIL,"test_obj_client_recv");

    c = client(allocator, CLIENT_TYPE_INET_UDP,
               (char *)"127.0.0.1", (char *)"8080");
    client_trustee(c, NULL, test_work_callback, NULL);
    
#if (defined(WINDOWS_USER_MODE))
    system("pause"); 
#else
    pause();
#endif
    object_destroy(c);
}
