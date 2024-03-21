/**
 * @file Mysql_Test.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, 
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Array_Stack.h>
#include <libobject/mockery/mockery.h>
#include "Mysql_Test.h"

static int __construct(Mysql_Test *test, char *init_str)
{
    allocator_t *allocator = test->parent.obj.allocator;
    Mysql *mysql;
    MYSQL *ret, *expect_ret = NULL;
    char sql_statement[1024];

    test->mysql = object_new(allocator, "Mysql", NULL);

    mysql = test->mysql;
    mysql->set(mysql, "/Sql/host", "127.0.0.1");
    mysql->set(mysql, "/Sql/user", "root");
    mysql->set(mysql, "/Sql/password", "root");
    mysql->set(mysql, "/Sql/database_name", "test");

    ret = test->mysql->connect(test->mysql);
    if (ret == NULL) {
        dbg_str(DBG_ERROR, "mysql_real_connect() failed!!, Error:%s", mysql_error(&test->mysql->mysql));
        mysql_close(&test->mysql->mysql);
    }

    sprintf(sql_statement,
            "CREATE TABLE IF NOT EXISTS test_user ("\
            "userID BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT, "\
            "nickname VARCHAR(255) NOT NULL, "\
            "mobile VARCHAR(255) NOT NULL, "\
            "password VARCHAR(255) NOT NULL, "\
            "signature VARCHAR(255), "\
            "portrait_url VARCHAR(255), "\
            "sex INT(11), "\
            "email VARCHAR(255), "\
            "balance BIGINT(20) UNSIGNED, "\
            "verification_code VARCHAR(255), "\
            "register_time DATETIME, "\
            "token VARCHAR(255), "\
            "PRIMARY KEY (userID))");

    ret = mysql->create_table(mysql, sql_statement);

    return 0;
}

static int __deconstruct(Mysql_Test *test)
{
    char sql[1024];

    sprintf(sql, "DROP TABLE test_user");
    test->mysql->drop_table(test->mysql, sql);
    test->mysql->close(test->mysql);

    dbg_str(DBG_ERROR,"deconstruct Mysql_Test");
    object_destroy(test->mysql);

    return 0;
}

static int __setup(Mysql_Test *test, char *init_str)
{
    return 1;
}

static int __teardown(Mysql_Test *test)
{
    return 1;
}

static int __test_connect(Mysql_Test *test)
{
    allocator_t *allocator = test->parent.obj.allocator;
    Sql *sql = (Sql *)test->mysql;
    MYSQL *ret, *expect_ret = NULL;

    dbg_str(DBG_DETAIL,"Mysql_Test set up");
    test->mysql->close(test->mysql);

    sql->set(sql, "/Sql/host", "127.0.0.1");
    sql->set(sql, "/Sql/user", "root");
    sql->set(sql, "/Sql/password", "root");
    sql->set(sql, "/Sql/database_name", "test");

    ret = test->mysql->connect(test->mysql);
    if (ret == NULL) {
        dbg_str(DBG_ERROR, "mysql_real_connect() failed!!, Error:%s", mysql_error(&test->mysql->mysql));
        mysql_close(&test->mysql->mysql);
    }

    ASSERT_NOT_EQUAL(test, &ret, &expect_ret, sizeof(ret));
}

static int __test_create_table(Mysql_Test *test)
{
    int ret, expect_ret = 1;
    Mysql *mysql = test->mysql;
    char sql[1024];

    /*create table*/
    sprintf(sql,
            "CREATE TABLE IF NOT EXISTS test_create_table ("\
            "userID BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT, "\
            "nickname VARCHAR(255) NOT NULL, "\
            "mobile VARCHAR(255) NOT NULL, "\
            "password VARCHAR(255) NOT NULL, "\
            "signature VARCHAR(255), "\
            "portrait_url VARCHAR(255), "\
            "sex INT(11), "\
            "email VARCHAR(255), "\
            "balance BIGINT(20) UNSIGNED, "\
            "verification_code VARCHAR(255), "\
            "register_time DATETIME, "\
            "token VARCHAR(255), "\
            "PRIMARY KEY (userID))");

    ret = mysql->create_table(mysql, sql);

    ASSERT_EQUAL(test, &ret, &expect_ret, sizeof(ret));
}

static int __test_drop_table(Mysql_Test *test)
{
    int ret, expect_ret = 1;
    Mysql *mysql = test->mysql;
    char sql[1024];

    sprintf(sql, "DROP TABLE test_create_table");
    ret = mysql->drop_table(mysql, sql);
    ASSERT_EQUAL(test, &ret, &expect_ret, sizeof(ret));

}

static int __test_insert(Mysql_Test *test)
{
    int ret, expect_ret = 1, previous_ret;
    Mysql *mysql = test->mysql;
    Result *result;
    int i, j, row_count, column_count;
    Row *r;
    Vector *rows;
    String *colomn;
    char *column1  = "userID";
    char *column2  = "nickname";
    char *column3  = "mobile";
    char *column4  = "password";
    char *column5  = "signature";
    char *column6  = "portrait_url";
    char *column7  = "sex";
    char *column8  = "email";
    char *column9  = "balance";
    char *column10 = "verification_code";
    char *column11 = "register_time";
    char sql[1024];
    char *nickname = "user";
    char *mobile = "13440129081";
    char *password = "123456";

    sprintf(sql, "SELECT * FROM test_user WHERE mobile=%s", mobile);
    previous_ret = mysql->query(mysql, sql);

    sprintf(sql, "INSERT INTO test_user (%s, %s, %s, %s) VALUES ('%s', '%s', '%s', '%s')",
            column2 ,column3, column4, column11,
            nickname, mobile, password, "2019-07-25 15:36:35");

    expect_ret = -1;
    ret = mysql->insert(mysql, sql);
    ASSERT_NUM_GREAT(test, ret, expect_ret);

    sprintf(sql, "SELECT * FROM test_user WHERE mobile=%s", mobile);
    ret = mysql->query(mysql, sql);
    previous_ret++;
    ASSERT_EQUAL(test, &ret, &previous_ret, sizeof(ret));
}

static int __test_del(Mysql_Test *test)
{
    int ret, expect_ret = 1, previous_ret;
    Mysql *mysql = test->mysql;
    char *column2  = "nickname";
    char *column3  = "mobile";
    char *column4  = "password";
    char *column11 = "register_time";
    char sql[1024];
    char *nickname = "tim";
    char *mobile = "13440129080";
    char *password = "123456";

    /*insert*/
    sprintf(sql, "INSERT INTO test_user (%s, %s, %s, %s) VALUES ('%s', '%s', '%s', '%s')",
            column2 ,column3, column4, column11,
            nickname, mobile, password, "2019-07-25 15:36:35");

    mysql->insert(mysql, sql);

    /*query*/
    sprintf(sql, "SELECT * FROM test_user WHERE mobile=%s", mobile);
    previous_ret = mysql->query(mysql, sql);
    dbg_str(DBG_DETAIL, "mobile row count=%d", previous_ret);

    /*del*/
    sprintf(sql, "DELETE FROM test_user WHERE mobile=%s", mobile);
    ret = mysql->del(mysql, sql);

    /*query*/
    sprintf(sql, "SELECT * FROM test_user WHERE mobile=%s", mobile);
    ret = mysql->query(mysql, sql);
    dbg_str(DBG_DETAIL, "mobile row count=%d, after del", ret);
    expect_ret = 0;
    ASSERT_EQUAL(test, &ret, &expect_ret, sizeof(ret));
}

static int __test_update(Mysql_Test *test)
{
    int ret, expect_ret = 1, previous_ret;
    Mysql *mysql = test->mysql;
    char *column2  = "nickname";
    char *column3  = "mobile";
    char *column4  = "password";
    char *column11 = "register_time";
    char sql[1024];
    char *nickname = "tim";
    char *mobile = "13440129082";
    char *password = "123456";

    /*del*/
    sprintf(sql, "DELETE FROM test_user WHERE nickname='%s'", "test_update_user");
    mysql->del(mysql, sql);

    /*insert*/
    sprintf(sql, "INSERT INTO test_user (%s, %s, %s, %s) VALUES ('%s', '%s', '%s', '%s')",
            column2 ,column3, column4, column11,
            nickname, mobile, password, "2019-07-25 15:36:35");
    mysql->insert(mysql, sql);

    /*update*/
    sprintf(sql, "UPDATE test_user SET %s='%s', %s='%s', %s='%s', %s='%s' WHERE mobile=%s",
            column2 ,"test_update_user", column3, mobile,
            column4, password, column11, "2019-07-25 15:36:35", mobile);
    ret = mysql->update(mysql, sql);

    /*query*/
    sprintf(sql, "SELECT * FROM test_user WHERE nickname='%s'", "test_update_user");
    ret = mysql->query(mysql, sql);
    dbg_str(DBG_DETAIL, "mobile row count=%d, after del", ret);
    expect_ret = 1;
    ASSERT_EQUAL(test, &ret, &expect_ret, sizeof(ret));

    /*del*/
    sprintf(sql, "DELETE FROM test_user WHERE nickname='%s'", "test_update_user");
    mysql->del(mysql, sql);
}

static int __test_query(Mysql_Test *test)
{
    int ret, expect_ret = 0;
    Mysql *mysql = test->mysql;
    Result *result;
    int i, j, row_count, column_count;
    Row *r;
    Vector *rows;
    String *colomn;
    char *column2  = "nickname";
    char *column3  = "mobile";
    char *column4  = "password";
    char *column11 = "register_time";
    char sql[1024];

    /*insert*/
    sprintf(sql, "INSERT INTO test_user (%s, %s, %s, %s) VALUES ('%s', '%s', '%s', '%s')",
            column2 ,column3, column4, column11,
            "alan0", "13440129080", "123456", "2019-07-25 15:36:35");
    mysql->insert(mysql, sql);

    /*query*/
    ret = mysql->query(mysql, "SELECT * FROM test_user");
    result = mysql->parent.result;
    row_count = result->get_row_count(result);
    column_count = result->get_column_count(result);
    rows = result->rows;


    dbg_str(DBG_DETAIL, "ret =%d", ret);
    dbg_str(DBG_DETAIL, "xcolumn_count =%d", column_count);
    for (i = 0; i < column_count; i++) {
        colomn = result->get_column(result, i);
        printf("%s ", colomn->get_cstr(colomn) == NULL ? "" : colomn->get_cstr(colomn));
    }
    printf("\n");

    for (i = 0; i < row_count; i++) {
        r = result->get_row(result, i);
        for (j = 0; r != NULL && j < column_count; j++) {
            colomn = r->get_column(r, j);
            if (colomn) {
                dbg_str(DBG_DETAIL, "colomn cache addr:%p", colomn->obj.cache);
                printf("%s ", colomn->get_cstr(colomn));
            }
        }
        printf("\n");
    }

    dbg_str(DBG_ERROR, "cache addr:%p", mysql->parent.cache);

    ASSERT_EQUAL(test, &ret, &row_count, sizeof(ret));
}

static class_info_entry_t mysql_test_class_info[] = {
    Init_Obj___Entry(0 , Test, parent),
    Init_Nfunc_Entry(1 , Mysql_Test, construct, __construct),
    Init_Nfunc_Entry(2 , Mysql_Test, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Mysql_Test, set, NULL),
    Init_Vfunc_Entry(4 , Mysql_Test, get, NULL),
    Init_Vfunc_Entry(5 , Mysql_Test, setup, __setup),
    Init_Vfunc_Entry(6 , Mysql_Test, teardown, __teardown),
    Init_Vfunc_Entry(7 , Mysql_Test, test_connect, __test_connect),
    Init_Vfunc_Entry(8 , Mysql_Test, test_create_table, __test_create_table),
    Init_Vfunc_Entry(9 , Mysql_Test, test_drop_table, __test_drop_table),
    Init_Vfunc_Entry(10, Mysql_Test, test_insert, __test_insert),
    Init_Vfunc_Entry(11, Mysql_Test, test_del, __test_del),
    Init_Vfunc_Entry(12, Mysql_Test, test_update, __test_update),
    Init_Vfunc_Entry(13, Mysql_Test, test_query, __test_query),
    Init_End___Entry(14, Mysql_Test),
};
REGISTER_CLASS("Mysql_Test", mysql_test_class_info);
