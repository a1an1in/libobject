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
#include <libobject/database/sql/Mysql_DB.h>
#include <libobject/mockery/mockery.h>

static int __test_mysql_insert_and_del(Mysql *mysql)
{
    int ret, expect_ret = 1, previous_ret;
    char *column2  = "nickname";
    char *column3  = "mobile";
    char *column4  = "password";
    char *column11 = "register_time";
    char sql[1024];
    char *nickname = "tim";
    char *mobile = "13440129080";
    char *password = "123456";

    TRY {
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
        THROW_IF(ret != expect_ret, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {}

    return ret;
}

static int __test_mysql_update(Mysql *mysql)
{
    int ret, expect_ret = 1, previous_ret;
    char *column2  = "nickname";
    char *column3  = "mobile";
    char *column4  = "password";
    char *column11 = "register_time";
    char sql[1024];
    char *nickname = "tim";
    char *mobile = "13440129082";
    char *password = "123456";

    TRY {
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
        THROW_IF(ret != expect_ret, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {
        /*del*/
        sprintf(sql, "DELETE FROM test_user WHERE nickname='%s'", "test_update_user");
        mysql->del(mysql, sql);
    }

    return ret;
}

static int __test_mysql_query(Mysql *mysql)
{
    int ret, expect_ret = 0;
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

    TRY {
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

        THROW_IF(ret != row_count, -1);
        dbg_str(DBG_SUC, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {}
    
    return ret;
}

static int test_mysql(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
    Mysql *mysql;
    char sql_statement[1024];
    int ret;

    TRY {
        mysql = object_new(allocator, "Mysql", NULL);

        mysql->set(mysql, "/Sql/host", "139.159.231.27");
        mysql->set(mysql, "/Sql/user", "root");
        mysql->set(mysql, "/Sql/password", "123456");
        mysql->set(mysql, "/Sql/database_name", "test");

        EXEC(mysql->connect(mysql));

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

        EXEC(mysql->create_table(mysql, sql_statement));

        EXEC(__test_mysql_insert_and_del(mysql));
        EXEC(__test_mysql_update(mysql));
        EXEC(__test_mysql_query(mysql));
    } CATCH (ret) {} FINALLY {
        sprintf(sql_statement, "DROP TABLE test_user");
        mysql->drop_table(mysql, sql_statement);
        mysql->close(mysql);
        object_destroy(mysql);
    }

    return ret;
}
REGISTER_TEST_CMD(test_mysql);