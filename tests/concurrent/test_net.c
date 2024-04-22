#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/concurrent/net/Inet_Udp_Client.h>
#include <libobject/concurrent/net/Inet_Tcp_Client.h>
#include <libobject/concurrent/net/Inet_Tcp_Server.h>
#include <libobject/concurrent/net/api.h>
#include <libobject/mockery/mockery.h>

static int test_net_result;
static int test_work_callback(void *task)
{
    work_task_t *t = (work_task_t *)task;

    if (t->buf_len == 0) { return 0; }
    dbg_str(DBG_SUC,"work callback, fd:%d,task len:%d content:%s", t->fd, t->buf_len, t->buf);
    if (strncmp("hello world", t->buf, t->buf_len) == 0) {
        test_net_result = 1;
    }
    
    return 1;
}

static int test_net_udp_ipv4(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    Client *c1 = NULL;
    Client *c2 = NULL;
    char *str = "hello world";
    int ret;

    TRY {
        c1 = client(allocator, CLIENT_TYPE_INET_UDP, (char *)"127.0.0.1", (char *)"1989");
        client_trustee(c1, NULL, test_work_callback, NULL);

        c2 = client(allocator, CLIENT_TYPE_INET_UDP, (char *)"127.0.0.1", (char *)"1990");
        client_connect(c2, "127.0.0.1", "1989");
        client_trustee(c2, NULL, NULL, NULL);
        usleep(1000);
        client_send(c2, str, strlen(str), 0);
        usleep(1000);
        THROW_IF(test_net_result != 1, 0);
    } CATCH (ret) {} FINALLY {
        object_destroy(c2);
        usleep(1000);
        object_destroy(c1);
        test_net_result = 0;
    }

    return ret;
}
REGISTER_TEST_FUNC(test_net_udp_ipv4);

#if (!defined(WINDOWS_USER_MODE))
static int test_net_udp_v6(TEST_ENTRY *entry, void *argc, void *argv)
{
    allocator_t *allocator = allocator_get_default_instance();
    char *localhost = "::1";
    Client *c1 = NULL;
    Client *c2 = NULL;
    char *str = "hello world";
    int ret;

    TRY {
        c1 = client(allocator, CLIENT_TYPE_INET_UDP_V6, localhost, (char *)"1989");
        client_trustee(c1, NULL, test_work_callback, NULL);

        c2 = client(allocator, CLIENT_TYPE_INET_UDP_V6, localhost, (char *)"1990");
        client_connect(c2, localhost, "1989");
        client_trustee(c2, NULL, NULL, NULL);
        usleep(1000);
        client_send(c2, str, strlen(str), 0);
        usleep(1000);
        THROW_IF(test_net_result != 1, 0);
    } CATCH (ret) {} FINALLY {
        object_destroy(c2);
        usleep(1000);
        object_destroy(c1);
        test_net_result = 0;
    }

    return ret;
}
// REGISTER_TEST_FUNC(test_net_udp_v6);
#endif

static int test_net_tcp_ipv4(TEST_ENTRY *entry, void *argc, void *argv)
{
    Server *s;
    allocator_t *allocator = allocator_get_default_instance();
    Client *c = NULL;
    char *str = "hello world";
    int ret;

    TRY {
        s = (Server *)server(allocator, SERVER_TYPE_INET_TCP, 
                             "127.0.0.1", "11013",
                             test_work_callback, s);
        c = client(allocator, CLIENT_TYPE_INET_TCP, NULL, NULL);
        client_connect(c, "127.0.0.1", "11013");
        client_trustee(c, NULL, NULL, NULL);
        
        usleep(1000);
        client_send(c, str, strlen(str), 0);
        usleep(1000);
        THROW_IF(test_net_result != 1, 0);
    } CATCH (ret) {} FINALLY {
        object_destroy(c);

        //这里需要等待C关闭， 不然s收到c关闭后可能删除两次worker，一次是析构中， 
        //一次在__new_conn_ev_callback。正常情况是不会出现这种情况的， 因为
        //server一般是不会释放的。
        usleep(1000); 
        server_destroy(s);
        test_net_result = 0;
    }

    return ret;
}
REGISTER_TEST_FUNC(test_net_tcp_ipv4);