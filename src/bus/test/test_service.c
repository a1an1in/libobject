#include <unistd.h>
#include <libobject/bus/bus.h>

static const struct blob_policy_s hello_policy[] = {
	[0] = { .name = "id", .type = BLOB_TYPE_INT32 },
	[1] = { .name = "content", .type = BLOB_TYPE_STRING },
};

static int test_hello(struct bus_s *bus,
                      int argc,
		      		  struct blob_attr_s **args,
                      void *out_data,
                      int *out_data_len)
{
    char buffer[] = {1,2,3,4,5,6,7,8,9};
    char *content;
    int id;

    dbg_str(DBG_SUC,"run test hello");

    id = blob_get_u32(args[0]);
    content = blob_get_string(args[1]);

    dbg_str(DBG_DETAIL,"test hello,rcv args id=%d,content=%s",id,content);

    memcpy(out_data,buffer,sizeof(buffer));
    *out_data_len = sizeof(buffer);

	return 1;
}

static const struct blob_policy_s set_policy[] = {
	[0] = { .name = "hijklmn", .type = BLOB_TYPE_INT32 },
	[1] = { .name = "opqrst", .type = BLOB_TYPE_STRING },
};

static int test_set(struct bus_s *bus,
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

static struct bus_object test_object = {
	.name      = (char *)"test",
	.methods   = (struct bus_method *)test_methods,
	.n_methods = ARRAY_SIZE(test_methods),
};
void test_bus_server()
{
    allocator_t *allocator = allocator_get_default_alloc();
    bus_t *bus;
#if 0
    char *deamon_host = "bus_server_path";
    char *deamon_srv = NULL;
#else
	/*
     *char *deamon_host = "192.168.20.49";
	 */
    char *deamon_host = "127.0.0.1";
    char *deamon_srv  = "12345";
#endif
	char buf[1024]    = "hello world!";
	int buf_len       = strlen(buf);
    
    dbg_str(DBG_DETAIL,"hello policy addr:%p",hello_policy);
    bus = bus_create(allocator,
                     deamon_host,
                     deamon_srv, 
                     CLIENT_TYPE_INET_TCP);

    dbg_str(BUS_DETAIL,"bus add object");
	bus_add_object(bus,&test_object);

    /*
	 *while(1) sleep(1);
     */
    pause();
    bus_destroy(bus);
}
