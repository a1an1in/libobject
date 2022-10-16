#ifndef __LIBOBJECT_MYSQL_H__
#define __LIBOBJECT_MYSQL_H__

#include <stdio.h>
#include <libobject/core/Obj.h>
#include <libobject/database/sql/Sql.h>
#include "mysql.h"

typedef struct Mysql_s Mysql;

struct Mysql_s{
    Sql parent;

    int (*construct)(Mysql *,char *);
    int (*deconstruct)(Mysql *);

    /*virtual methods reimplement*/
    int (*set)(Mysql *sql, char *attrib, void *value);
    void *(*get)(Mysql *, char *attrib);
    int (*connect)(Mysql *sql);
    int (*create_table)(Mysql *sql, char *sql_str);
    int (*drop_table)(Mysql *sql, char *sql_str);
    int (*insert)(Mysql *sql, char *sql_str);
    int (*del)(Mysql *sql, char *sql_str);
    int (*update)(Mysql *sql, char *sql_str);
    int (*query)(Mysql *sql, char *sql_str);
    int (*close)(Mysql *sql);

    /*attribs*/
    MYSQL mysql;
};

#endif
