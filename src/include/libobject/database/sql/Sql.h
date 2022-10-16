#ifndef __SQL_H__
#define __SQL_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/core/Object_Cache.h>
#include <libobject/core/String.h>
#include <libobject/database/sql/Result.h>

typedef struct Sql_s Sql;

struct Sql_s{
    Obj parent;

    int (*construct)(Sql *,char *);
    int (*deconstruct)(Sql *);

    /*virtual methods reimplement*/
    int (*set)(Sql *sql, char *attrib, void *value);
    void *(*get)(Sql *, char *attrib);
    int (*connect)(Sql *sql);
    int (*create_table)(Sql *sql, char *sql_str);
    int (*drop_table)(Sql *sql, char *sql_str);
    int (*insert)(Sql *sql, char *sql_str);
    int (*del)(Sql *sql, char *sql_str);
    int (*update)(Sql *sql, char *sql_str);
    int (*query)(Sql *sql, char *sql_str);
    int (*close)(Sql *sql);

    /*attribs*/
    String *host;
    String *user;
    String *password;
    String *port;
    String *database_name;
    Object_Cache *cache;
    Result *result;
};

#endif
