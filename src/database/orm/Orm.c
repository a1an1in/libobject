/**
 * @file Orm.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-07-25
 */

#include <libobject/database/orm/Orm.h>

static int __construct(Orm *orm, char *init_str)
{
    allocator_t *allocator = orm->parent.allocator;
    uint8_t trustee_flag = 1;
    int value_type = VALUE_TYPE_OBJ_POINTER;
    Queue *conns;

    orm->host          = object_new(allocator, "String", NULL);
    orm->user          = object_new(allocator, "String", NULL);
    orm->password      = object_new(allocator, "String", NULL);
    orm->port          = object_new(allocator, "String", NULL);
    orm->database_name = object_new(allocator, "String", NULL);
    conns              = object_new(allocator, "Linked_Queue", NULL);

    conns->set(conns, "/Queue/value_type", &value_type);
    conns->set(conns, "/Queue/trustee_flag", &trustee_flag);
    orm->conns = conns;

    orm->max_conn_count = 10;
    orm->default_conn_count = 1;

    return 1;
}

static int __deconstruct(Orm *orm)
{
    object_destroy(orm->host);
    object_destroy(orm->user);
    object_destroy(orm->password);
    object_destroy(orm->port);
    object_destroy(orm->database_name);
    object_destroy(orm->conns);

    return 1;
}

static int __register_table(Orm *orm, char *table_name)
{
    return 1;
}

static int __run(Orm *orm)
{
    int i, conn_count;
    allocator_t *allocator = orm->parent.allocator;
    Orm_Conn *conn;
    char *host, *user, *password, *database_name;
    int ret;

    conn_count = orm->default_conn_count;
    host = orm->host->get_cstr(orm->host);
    user = orm->user->get_cstr(orm->user);
    password = orm->password->get_cstr(orm->password);
    database_name = orm->database_name->get_cstr(orm->database_name);

    dbg_str(DB_DETAIL, "conn_count=%d", conn_count);
    for (i = 0; i < conn_count; i++) {
        conn = object_new(allocator, "Orm_Conn", NULL);
        conn->open(conn, host, user, password, database_name);
        orm->conns->add_back(orm->conns, conn);
    }

    return ret;
}

static Orm_Conn * __get_conn(Orm *orm)
{
    Orm_Conn *conn = NULL;

    orm->conns->remove(orm->conns, (void **)&conn);

    //err processing ...
    
    return conn;
}

static int __add_conn(Orm *orm, Orm_Conn *conn)
{
    return orm->conns->add_back(orm->conns, conn);
}

static class_info_entry_t orm_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Orm, construct, __construct),
    Init_Nfunc_Entry(2 , Orm, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Orm, set, NULL),
    Init_Vfunc_Entry(4 , Orm, get, NULL),
    Init_Vfunc_Entry(5 , Orm, register_table, __register_table),
    Init_Vfunc_Entry(6 , Orm, run, __run),
    Init_Vfunc_Entry(7 , Orm, get_conn, __get_conn),
    Init_Vfunc_Entry(8 , Orm, add_conn, __add_conn),
    Init_Str___Entry(9 , Orm, host, NULL),
    Init_Str___Entry(10, Orm, user, NULL),
    Init_Str___Entry(11, Orm, password, NULL),
    Init_Str___Entry(12, Orm, port, NULL),
    Init_Str___Entry(13, Orm, database_name, NULL),
    Init_U32___Entry(14, Orm, max_conn_count, NULL),
    Init_U32___Entry(15, Orm, default_conn_count, NULL),
    Init_End___Entry(16, Orm),
};
REGISTER_CLASS(Orm, orm_class_info);

