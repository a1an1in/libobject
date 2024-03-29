#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/concurrent/message/Publisher.h> 
#include <libobject/concurrent/message/Centor.h>
#include <libobject/concurrent/message/Subscriber.h>
#include <libobject/mockery/mockery.h>

static int got_on_pause_message_flag;

static void test_on_pause(message_t *message, void *arg)
{
    Subscriber *subscriber = (Subscriber *)arg;
    dbg_str(DBG_VIP, "subscriber receive a message:%s", (char *)message->what);
    dbg_str(DBG_DETAIL, "message arg:%p", arg);
    got_on_pause_message_flag = 1;
}

static void test_xxxx(message_t *message, void *arg)
{
    Subscriber *subscriber = (Subscriber *)arg;
    dbg_str(DBG_VIP, "subscriber receive a message:%s", (char *)message->what);
    dbg_str(DBG_DETAIL, "message arg:%p", arg);
}

static int test_message_publisher()
{
    Centor *centor;
    Publisher *publisher;
    Subscriber *subscriber;
    allocator_t *allocator = allocator_get_default_instance();
    char * test_str = "on_pause";
    int ret;

    TRY {
        centor     = OBJECT_NEW(allocator, Centor, NULL);
        publisher  = OBJECT_NEW(allocator, Publisher, NULL);
        subscriber = OBJECT_NEW(allocator, Subscriber, NULL);

        subscriber->connect_centor(subscriber, centor);
        subscriber->add_method(subscriber, "on_pause", test_on_pause, allocator);
        subscriber->add_method(subscriber, "test_xxxx", test_xxxx, allocator);   
        subscriber->subscribe(subscriber, publisher);

        dbg_str(DBG_VIP, "%p subscribe a publisher, publisher addr:%p", subscriber, publisher);
        dbg_str(DBG_VIP, "message handler arg:%p", allocator);

        publisher->connect_centor(publisher, centor);
        publisher->publish_message(publisher, test_str, strlen(test_str));

        usleep(1 * 1000 * 1000);
        THROW_IF(got_on_pause_message_flag != 1, -1);
    } CATCH (ret) {} FINALLY {
        object_destroy(subscriber);
        object_destroy(publisher);
        object_destroy(centor);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_message_publisher);