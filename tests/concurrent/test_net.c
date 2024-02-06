#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/concurrent/net/Inet_Udp_Client.h>
#include <libobject/concurrent/net/Inet_Tcp_Client.h>
#include <libobject/concurrent/net/Inet_Tcp_Server.h>
#include <libobject/concurrent/net/api.h>

static int test_work_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;

    if (t->buf_len == 0) { return 0; }
    dbg_str(NET_SUC,"task len:%d content:%s", t->buf_len, t->buf);
    
    return 1;
}

static int test_client_udp_ipv4_recv(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    Client *c = NULL;

    dbg_str(NET_DETAIL,"test_obj_client_recv");

    c = client(allocator, CLIENT_TYPE_INET_UDP, (char *)"127.0.0.1", (char *)"1989");
    client_trustee(c, NULL, test_work_callback, NULL);
#if (defined(WINDOWS_USER_MODE))
    system("pause"); 
#else
    pause();
#endif
    object_destroy(c);
}
REGISTER_TEST_CMD(test_client_udp_ipv4_recv);

static int test_client_udp_ipv4_send(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    Client *c = NULL;
    char *str = "hello world";

    dbg_str(NET_DETAIL,"test_obj_client_send");

    c = client(allocator, CLIENT_TYPE_INET_UDP, (char *)"127.0.0.1", (char *)"1990");
    client_connect(c, "127.0.0.1", "1989");
    client_trustee(c, NULL, test_work_callback, NULL);
    client_send(c, str, strlen(str), 0);

    object_destroy(c);
    dbg_str(NET_WARNNING,"test_obj_client_send end");
}
REGISTER_TEST_CMD(test_client_udp_ipv4_send);

static int test_client_inet_tcp_ipv4(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    Client *c = NULL;
    char *str = "hello world";
    int ret;

    TRY {
        dbg_str(NET_DETAIL, "test_obj_client_send");

        c = client(allocator, CLIENT_TYPE_INET_TCP, NULL, NULL);
        client_connect(c, "127.0.0.1", "11013");
        client_trustee(c, NULL, test_work_callback, NULL);
        client_send(c, str, strlen(str), 0);
    } CATCH (ret) {} FINALLY {
        object_destroy(c);
    }

    return ret;
}
REGISTER_TEST_CMD(test_client_inet_tcp_ipv4);

static int test_server_inet_tcp_ipv4(TEST_ENTRY *entry, void *argc, void *argv)
{
    Server *s;
    allocator_t *allocator = allocator_get_default_instance();
    int pre_alloc_count, after_alloc_count;
    int ret;

    TRY {
        sleep(1);
        dbg_str(DBG_SUC, "test_server_inet_tcp_ipv4");
        pre_alloc_count = allocator->alloc_count;
        s = (Server *)server(allocator, SERVER_TYPE_INET_TCP, 
                             "127.0.0.1", "11013",
                             test_work_callback, s);

#if (defined(WINDOWS_USER_MODE))
        sleep(1000);
#else
        sleep(10);
#endif
    } CATCH (ret) {} FINALLY {
        server_destroy(s);
        after_alloc_count = allocator->alloc_count;
        ret = assert_equal(&pre_alloc_count, &after_alloc_count, sizeof(int));
        if (ret == 0) {
            dbg_str(NET_WARNNING, "server has memory omit, pre_alloc_count=%d, after_alloc_count=%d",
                    pre_alloc_count, after_alloc_count);
            ret = -1;
        }
    }

    return ret;
}
REGISTER_TEST_CMD(test_server_inet_tcp_ipv4);

static int test_client_udp_v6_recv(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    char *localhost = "::1";
    Client *c = NULL;

    dbg_str(NET_DETAIL,"test_obj_client_recv");

    c = client(allocator, CLIENT_TYPE_INET_UDP_V6, localhost, (char *)"1989");
    client_trustee(c, NULL, test_work_callback, NULL);
#if (defined(WINDOWS_USER_MODE))
    system("pause"); 
#else
    pause();
#endif
    object_destroy(c);
}
REGISTER_TEST_CMD(test_client_udp_v6_recv);

static int test_client_udp_v6_send(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    Client *c = NULL;
    char *str = "hello world";
    char *localhost = "::1";

    dbg_str(NET_DETAIL,"test_obj_client_send");

    c = client(allocator, CLIENT_TYPE_INET_UDP_V6, localhost, (char *)"1990");
    client_connect(c, localhost, "1989");
    client_trustee(c, NULL, test_work_callback, NULL);
    client_send(c, str, strlen(str), 0);

    object_destroy(c);
    dbg_str(NET_WARNNING,"test_obj_client_send end");
}
REGISTER_TEST_CMD(test_client_udp_v6_send);