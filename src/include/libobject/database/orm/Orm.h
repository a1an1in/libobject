#ifndef __ORM_H__
#define __ORM_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/String.h>
#include <libobject/core/Queue.h>
#include <libobject/database/orm/Orm_Conn.h>
#include <libobject/database/orm/Table.h>

typedef struct Orm_s Orm;

struct Orm_s{
    Obj parent;

    int (*construct)(Orm *,char *);
    int (*deconstruct)(Orm *);

    /*virtual methods reimplement*/
    int (*set)(Orm *orm, char *attrib, void *value);
    void *(*get)(Orm *, char *attrib);
    int (*register_table)(Orm *orm, char *table_name);
    int (*run)(Orm *orm);
    Orm_Conn *(*get_conn)(Orm *orm);
    int (*add_conn)(Orm *orm, Orm_Conn *conn);

    /*attribs*/
    String *host;
    String *user;
    String *password;
    String *port;
    String *database_name;
    Queue *conns;
    int max_conn_count;
    int default_conn_count;
};

#endif
