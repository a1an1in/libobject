#include <stdio.h>
#include <stdlib.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/database/sql/Sql.h>
#include "mysql.h"

static int __construct(Sql *sql, char *init_str)
{
    allocator_t *allocator = sql->parent.allocator;

    sql->host          = object_new(allocator, "String", NULL);
    sql->user          = object_new(allocator, "String", NULL);
    sql->password      = object_new(allocator, "String", NULL);
    sql->port          = object_new(allocator, "String", NULL);
    sql->database_name = object_new(allocator, "String", NULL);
    sql->cache         = object_new(allocator, "Object_Cache", NULL);
    sql->result        = object_new(allocator, "Result", NULL);

    return 0;
}

static int __deconstruct(Sql *sql)
{
    if (sql->host)
        object_destroy(sql->host);

    if (sql->user)
        object_destroy(sql->user);

    if (sql->password)
        object_destroy(sql->password);

    if (sql->port)
        object_destroy(sql->port);

    if (sql->database_name)
        object_destroy(sql->database_name);

    if (sql->result) 
        object_destroy(sql->result);

    if (sql->cache)
        object_destroy(sql->cache);

    return 0;
}


static class_info_entry_t mysql_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Sql, construct, __construct),
    Init_Nfunc_Entry(2 , Sql, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Sql, set, NULL),
    Init_Vfunc_Entry(4 , Sql, get, NULL),
    Init_Vfunc_Entry(5 , Sql, connect, NULL),
    Init_Vfunc_Entry(6 , Sql, create_table, NULL),
    Init_Vfunc_Entry(7 , Sql, drop_table, NULL),
    Init_Vfunc_Entry(8 , Sql, insert, NULL),
    Init_Vfunc_Entry(9 , Sql, del, NULL),
    Init_Vfunc_Entry(10, Sql, update, NULL),
    Init_Vfunc_Entry(11, Sql, query, NULL),
    Init_Vfunc_Entry(12, Sql, close, NULL),
    Init_Str___Entry(13, Sql, host, NULL),
    Init_Str___Entry(14, Sql, user, NULL),
    Init_Str___Entry(15, Sql, password, NULL),
    Init_Str___Entry(16, Sql, port, NULL),
    Init_Str___Entry(17, Sql, database_name, NULL),
    Init_End___Entry(18, Sql),
};
REGISTER_CLASS(Sql, mysql_class_info);
