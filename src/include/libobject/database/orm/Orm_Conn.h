#ifndef __ORM_SESSION_H__
#define __ORM_SESSION_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/database/sql/Sql.h>
#include <libobject/database/orm/Table.h>
#include <libobject/database/orm/Model.h>

typedef struct Orm_Conn_s Orm_Conn;

struct Orm_Conn_s{
    Obj parent;

    int (*construct)(Orm_Conn *,char *);
    int (*deconstruct)(Orm_Conn *);

    /*virtual methods reimplement*/
    int (*set)(Orm_Conn *connection, char *attrib, void *value);
    void *(*get)(Orm_Conn *, char *attrib);

    int (*open)(Orm_Conn *connection, char *host, char *user, char *password, char *database_name);
    int (*create_table)(Orm_Conn *connection, char *table_class_name);
    int (*drop_table)(Orm_Conn *connection, char *table_class_name);
    int (*close)(Orm_Conn *connection);

    int (*insert_model)(Orm_Conn *connection, Model *model);
    int (*insert_table)(Orm_Conn *connection, Table *table);
    int (*del)(Orm_Conn *connection, char *sql_statement, ...);
    int (*update_model)(Orm_Conn *connection, Model *model);
    int (*update_table)(Orm_Conn *connection, Table *table);
    int (*update)(Orm_Conn *connection, char *model_name, char *json);
    int (*query_model)(Orm_Conn *connection, Model *model, char *sql_statement, ...);
    int (*query_table)(Orm_Conn *conn, Table *table, char *sql_fmt, ...);
    int (*__insert_or_update_model)(Orm_Conn *connection, Model *model);
    int (*insert_or_update_table)(Orm_Conn *connection, Table *table);

    /*attribs*/
    Sql *sql;
};

#endif
