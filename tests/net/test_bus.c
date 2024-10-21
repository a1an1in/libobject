
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/net/bus/bus.h>
#include <libobject/net/bus/busd.h>
#include <libobject/mockery/mockery.h>

static const struct blob_policy_s hello_policy[] = {
	[0] = { .name = "id", .type = BLOB_TYPE_UINT32 },
	[1] = { .name = "content", .type = BLOB_TYPE_STRING },
};

static int test_hello(bus_object_t *obj,
                      int argc,
		      		  struct blob_attr_s **args,
                      void *out_data,
                      int *out_data_len)
{
    char buffer[] = {1,2,3,4,5,6,7,8,9};
    char *content;
    int id;

    id = blob_get_uint32(args[0]);
    content = blob_get_string(args[1]);

    dbg_str(DBG_DETAIL,"test hello,rcv args id=%d,content=%s",id,content);

    memcpy(out_data, buffer, sizeof(buffer));
    *out_data_len = sizeof(buffer);

	return 1;
}

static const struct blob_policy_s set_policy[] = {
	[0] = { .name = "hijklmn", .type = BLOB_TYPE_UINT32 },
	[1] = { .name = "opqrst", .type = BLOB_TYPE_STRING },
};

static int test_set(bus_object_t *obj,
                    int argc,
		      		struct blob_attr_s **args,
                    void *out_data,
                    int *out_data_len)
{
    printf("run set set\n");
	return 0;
}

static const struct bus_method test_methods[] = {
	BUS_METHOD("hello", test_hello, hello_policy),
	BUS_METHOD("set", test_set, set_policy),
};

static bus_object_t test_object = {
    .cname     = (char *)"bus_test_class", 
	.methods   = (struct bus_method *)test_methods,
	.n_methods = ARRAY_SIZE(test_methods),
};


static int __test_bus_invoke_sync()
{
    allocator_t *allocator = allocator_get_default_instance();
    bus_t *bus;
    char *deamon_host = "127.0.0.1";
    char *deamon_srv  = "12345";
	char out[1024] = {0};
    int out_len = sizeof(out);
    char expert_buffer[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    bus_method_args_t args[2] = {
        [0] = {ARG_TYPE_UINT32, "id", 123},
        [1] = {ARG_TYPE_STRING, "content", "hello_world"},
    };
    int ret;

    TRY {
        dbg_str(DBG_VIP, "test_bus_invoke_sync");
        bus = bus_create(allocator, deamon_host, deamon_srv, CLIENT_TYPE_INET_TCP);
        THROW_IF(bus == NULL, -1);

        EXEC(bus_invoke_sync(bus, "test", "hello", 2, args, out, &out_len));
        dbg_buf(DBG_VIP, "return buffer:", (uint8_t *)out, out_len);
        THROW_IF(assert_equal(out, expert_buffer, sizeof(expert_buffer)) != 1, -1);
        dbg_str(DBG_VIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {
        bus_destroy(bus);
    }

    return ret;
}

static int __test_bus_lookup_one_sync()
{
    allocator_t *allocator = allocator_get_default_instance();
    bus_t *bus;
    char *deamon_host = "127.0.0.1";
    char *deamon_srv  = "12345";
	char out[1024];
    int out_len = 1024;
    int ret;

    TRY {
        bus = bus_create(allocator, deamon_host, deamon_srv, CLIENT_TYPE_INET_TCP);
        THROW_IF(bus == NULL, -1);
        bus_lookup_sync(bus, "test", out, &out_len);

        THROW_IF(strstr(out, "test") == NULL, -1);
        dbg_str(DBG_VIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {
        bus_destroy(bus);
    }
	
    return ret;
}

static int __test_bus_lookup_all_sync()
{
    allocator_t *allocator = allocator_get_default_instance();
    bus_t *bus;
    char *deamon_host = "127.0.0.1";
    char *deamon_srv  = "12345";
	char out[1024];
    int out_len = 1024;
    int ret;

    TRY {
        bus = bus_create(allocator, deamon_host, deamon_srv, CLIENT_TYPE_INET_TCP);
        THROW_IF(bus == NULL, -1);
        bus_lookup_sync(bus, "all", out, &out_len);

        THROW_IF(strstr(out, "all") == NULL, -1);
        dbg_str(DBG_VIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {
        bus_destroy(bus);
    }
	
    return ret;
}

static int test_bus()
{
    allocator_t *allocator = allocator_get_default_instance();
    bus_t *bus = NULL;
    busd_t *busd = NULL;
    char *deamon_host = "127.0.0.1";
    char *deamon_srv  = "12345";
	char buf[1024]    = "hello world!";
	int buf_len       = strlen(buf);
    int ret, count = 0;
    
    TRY {
        sleep(1);
        busd = busd_create(allocator, deamon_host,
                           deamon_srv, SERVER_TYPE_INET_TCP);
        THROW_IF(busd == NULL, -1);

        bus = bus_create(allocator, deamon_host,
                         deamon_srv, CLIENT_TYPE_INET_TCP);
        THROW_IF(bus == NULL, -1);

        memcpy(test_object.id, "test", strlen("test"));
        bus_add_object(bus, &test_object);

        EXEC(__test_bus_invoke_sync());
        // EXEC(__test_bus_lookup_one_sync());
        EXEC(__test_bus_lookup_all_sync());
    } CATCH (ret) {} FINALLY {
        bus_destroy(bus);
        usleep(1000); 
        busd_destroy(busd);
    }

    return ret;
}
REGISTER_TEST_FUNC(test_bus);