#include <stdio.h>
#include <stdlib.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/database/sql/Mysql_DB.h>
#include <libobject/database/sql/Row.h>
#include <libobject/database/sql/Result.h>
#include <errno.h>

/*
 * 判断 MySQL 错误码是否为连接断开类错误。
 * 这类错误意味着服务端已经断开连接，需要释放旧连接并重新建立。
 */
static int __is_connection_error(Mysql *mysql)
{
    unsigned int err_no = mysql_errno(&mysql->mysql);

    switch (err_no) {
    case CR_SERVER_LOST:
    case CR_SERVER_GONE_ERROR:
    case CR_CONNECTION_ERROR:
    case CR_CONN_HOST_ERROR:
        return 1;
    default:
        return 0;
    }
}

/* 前向声明，供 __reconnect 调用 */
static int __connect(Mysql *mysql);

/*
 * 释放旧连接，重新初始化 MYSQL 句柄，然后复用 __connect 建立新连接。
 */
static int __reconnect(Mysql *mysql)
{
    dbg_str(DB_WARN, "connection lost, closing old and reconnecting...");

    mysql_close(&mysql->mysql);
    mysql_init(&mysql->mysql);

    return __connect(mysql);
}

static int __construct(Mysql *sql, char *init_str)
{
    if (mysql_library_init(0, NULL, NULL)) {
        fprintf(stderr, "could not initialize MySQL client library\n");
        exit(1);
    }

    mysql_init(&sql->mysql);

    return 0;
}

static int __deconstruct(Mysql *sql)
{
    mysql_library_end();

    return 0;
}

static int __connect(Mysql *mysql)
{
    Sql *sql = (Sql *)mysql;
    MYSQL *ret;
    unsigned int port_num = 0;
    unsigned long client_flag = 0;

    if (sql->port != NULL) {
        port_num = atoi(sql->port->get_cstr(sql->port));
    }

    ret = mysql_real_connect(&mysql->mysql,
                             sql->host->get_cstr(sql->host),
                             sql->user->get_cstr(sql->user),
                             sql->password->get_cstr(sql->password),
                             sql->database_name->get_cstr(sql->database_name),
                             port_num,
                             NULL,
                             client_flag);
    if (ret == NULL) {
        dbg_str(DB_ERROR, "mysql_real_connect() failed!!, Error:%s",
                mysql_error(&mysql->mysql));
        dbg_str(DB_ERROR, "host:%s, user:%s, password:%s, databse_name:%s",
                sql->host->get_cstr(sql->host),
                sql->user->get_cstr(sql->user),
                sql->password->get_cstr(sql->password),
                sql->database_name->get_cstr(sql->database_name));
        mysql_close(&mysql->mysql);
        return -1;
    }

    return 1;
}

static int __close(Mysql *mysql)
{
    mysql_close(&mysql->mysql);
}

/*
 * 执行 mysql_query，如果失败且是连接断开错误，则自动重连后重试一次。
 * 返回 0 成功，-1 失败。
 */
static int __exec_query_with_retry(Mysql *mysql, char *sql_statement)
{
    int ret;

    ret = mysql_query(&mysql->mysql, sql_statement);
    if (ret == 0)
        return 0;

    /*
     * 如果查询失败且是连接断开类错误，释放旧连接、重建新连接后重试一次。
     */
    if (__is_connection_error(mysql)) {
        dbg_str(DB_WARN, "query failed due to connection lost, try reconnect. Error:%s",
                mysql_error(&mysql->mysql));

        if (__reconnect(mysql) == 1) {
            ret = mysql_query(&mysql->mysql, sql_statement);
            if (ret == 0)
                return 0;
        }
    }

    dbg_str(DB_ERROR, "mysql_query() failed!!, Error:%s",
            mysql_error(&mysql->mysql));
    return -1;
}

static int __create_table(Mysql *mysql, char *sql_statement)
{
    int ret;

    ret = __exec_query_with_retry(mysql, sql_statement);
    if (ret) {
        dbg_str(DB_ERROR, "create_table() failed!!, Error:%s",
                mysql_error(&mysql->mysql));
        mysql_close(&mysql->mysql);
        return -1;
    }

    return 1;
}

static int __drop_table(Mysql *mysql, char *sql_statement)
{
    int ret;

    ret = __exec_query_with_retry(mysql, sql_statement);
    if (ret) {
        dbg_str(DB_ERROR, "drop_table() failed!!, Error:%s",
                mysql_error(&mysql->mysql));
        mysql_close(&mysql->mysql);
        return -1;
    }

    return 1;
}

static int __insert(Mysql *mysql, char *sql_statement)
{
    int ret;

    ret = __exec_query_with_retry(mysql, sql_statement);
    if (ret) {
        dbg_str(DB_ERROR, "insert() failed!!, Error:%s",
                mysql_error(&mysql->mysql));
        mysql_close(&mysql->mysql);
        return -1;
    }

    return 1;
}

static int __del(Mysql *mysql, char *sql_statement)
{
    int ret;

    ret = __exec_query_with_retry(mysql, sql_statement);
    if (ret) {
        dbg_str(DB_ERROR, "del() failed!!, Error:%s",
                mysql_error(&mysql->mysql));
        mysql_close(&mysql->mysql);
        return -1;
    }

    return 1;
}

static int __update(Mysql *mysql, char *sql_statement)
{
    int ret;

    ret = __exec_query_with_retry(mysql, sql_statement);
    if (ret) {
        dbg_str(DB_ERROR, "update() failed!!, Error:%s",
                mysql_error(&mysql->mysql));
        mysql_close(&mysql->mysql);
        return -1;
    }

    return 0;
}

static int __query(Mysql *mysql, char *sql_statement)
{
    Object_Cache *cache = mysql->parent.cache;
    MYSQL_RES *result;
    MYSQL_ROW row;
    unsigned int num_fields;
    unsigned int i;
    Result *res = mysql->parent.result;
    Row *r;
    String *column;
    int ret, row_count = 0;
    unsigned long *lengths;

    ret = __exec_query_with_retry(mysql, sql_statement);
    if (ret) {
        dbg_str(DB_ERROR, "query() failed!!, Error:%s",
                mysql_error(&mysql->mysql));
        mysql_close(&mysql->mysql);
        return -1;
    }

    do {
        res->reset(res);
        result = mysql_store_result(&mysql->mysql);
        if (result) {
            num_fields = mysql_num_fields(result);

            MYSQL_FIELD *fields = mysql_fetch_fields(result);
            if (!fields) {
                dbg_str(DB_ERROR, "MySQL fields fetch is error!");
                return -1;
            }

            for (int i = 0; i < num_fields; i++) {
                column = cache->new(cache, "String", fields[i].name);
                res->set_column(res, i, column);
            }

            while ((row = mysql_fetch_row(result))) {
                r = cache->new(cache, "Row", NULL);
                
                lengths = mysql_fetch_lengths(result);
                for (i = 0; i < num_fields; i++) {
                    if (row[i] == NULL) {
                        column = NULL;
                    } else {
                        column = cache->new(cache, "String", row[i]);
                    }
                    r->set_column(r, i, column);
                }
                res->set_row_at_back(res, r);
                row_count++;
            }
            mysql_free_result(result);
            mysql->parent.result = res;
        } else {
            if (mysql_field_count(&mysql->mysql) == 0) {
                printf("lld rows affected\n");
                break;
            }
        }
        if ((ret = mysql_next_result(&mysql->mysql)) > 0) {
            printf("Counld not execute statement\n");
        }
    } while (ret == 0);

    return row_count;
}

static class_info_entry_t mysql_class_info[] = {
    Init_Obj___Entry(0 , Sql, parent),
    Init_Nfunc_Entry(1 , Mysql, construct, __construct),
    Init_Nfunc_Entry(2 , Mysql, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Mysql, set, NULL),
    Init_Vfunc_Entry(4 , Mysql, get, NULL),
    Init_Vfunc_Entry(5 , Mysql, connect, __connect),
    Init_Vfunc_Entry(6 , Mysql, close, __close),
    Init_Vfunc_Entry(7 , Mysql, create_table, __create_table),
    Init_Vfunc_Entry(8 , Mysql, drop_table, __drop_table),
    Init_Vfunc_Entry(9 , Mysql, insert, __insert),
    Init_Vfunc_Entry(10, Mysql, del, __del),
    Init_Vfunc_Entry(11, Mysql, update, __update),
    Init_Vfunc_Entry(12, Mysql, query, __query),
    Init_End___Entry(13, Mysql),
};
REGISTER_CLASS(Mysql, mysql_class_info);
