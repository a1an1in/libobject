/**
 * @file Orm_Test.c
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
#include <libobject/core/try.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/Array_Stack.h>
#include <libobject/core/utils/data_structure/bitmap.h>
#include <libobject/database/orm/Orm.h>
#include <libobject/mockery/mockery.h>
#include "Test_User_Model.h"

static int __test_orm_create_table(Orm *orm)
{
    Orm_Conn *conn;
    int ret;

    conn = orm->get_conn(orm);
    if (conn) {
        ret = conn->create_table(conn, "Test_User_Table"); /* 专用于测试创建表 */
    }
    orm->add_conn(orm, conn);
    dbg_str(DBG_VIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);

    return ret;
}

static int __test_orm_model_operations(Orm *orm)
{
    Orm_Conn *conn;
    int count, i, expect_count = 1, ret = 1, id = 0;
    Test_User_Model *user = NULL;
    int age = 22;

    TRY {
        conn = orm->get_conn(orm);
        THROW_IF(conn == NULL, -1);
        EXEC(conn->del(conn, "DELETE FROM test_user_table"));
        user  = object_new(orm->parent.allocator, "Test_User_Model", NULL);

        /* 新增 */
        user->set(user, "nickname", "user1");
        user->set(user, "mobile", "13440129081");
        user->set(user, "password", "123456");
        user->set(user, "age", &age);
        conn->insert_model(conn, (Model *)user);

        user->set(user, "nickname", "user2");
        user->set(user, "mobile", "13440129082");
        user->set(user, "password", "123456");
        user->set(user, "age", &age);
        conn->insert_model(conn, (Model *)user);

        /* 验证是否新增成功 */
        EXEC(conn->query_model(conn, (Model *)user, "select * from test_user_table where nickname='%s';", "user1"));
        dbg_str(DBG_DETAIL, "json:%s", user->to_json(user));
        THROW_IF(strcmp(STR2A(user->mobile), "13440129081") != 0, -1);

        /* update */
        user->set(user, "nickname", "user1");
        user->set(user, "mobile", "15440129083");
        user->set(user, "password", "123456");
        user->set(user, "age", &age);
        EXEC(conn->update_model(conn, (Model *)user));

        /* 验证是否修改成功 */
        EXEC(conn->query_model(conn, (Model *)user, "select * from test_user_table where nickname='%s';", "user1"));
        dbg_str(DBG_DETAIL, "json:%s", user->to_json(user));
        THROW_IF(strcmp(STR2A(user->mobile), "15440129083") != 0, -1);
        dbg_str(DBG_VIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);

        /* 测试update with json */
        user->set(user, "nickname", "user1");
        user->set(user, "mobile", "15440129084");
        user->set(user, "password", "123456");
        user->set(user, "age", &age);
        EXEC(conn->update(conn, "UPDATE test_user_table SET mobile='15440129084' WHERE nickname='user1'"));

        /* 验证是否修改成功 */
        EXEC(conn->query_model(conn, (Model *)user, "select * from test_user_table where nickname='%s';", "user1"));
        THROW_IF(strcmp(STR2A(user->mobile), "15440129084") != 0, -1);
        dbg_str(DBG_VIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);

    } CATCH (ret) { } FINALLY {
        object_destroy(user);
        orm->add_conn(orm, conn);
    }

    return ret;
}

static int __test_orm_table_operations(Orm *orm)
{
    Orm_Conn *conn;
    Test_User_Model *user;
    Test_User_Model *test_user;
    Table *table = NULL;
    int age = 22;
    int ret, count;

    TRY {
        dbg_str(DBG_DETAIL, "test_insert_table");
        user = object_new(orm->parent.allocator, "Test_User_Model", NULL);
        user->set(user, "nickname", "user1");
        user->set(user, "mobile", "13440129080");
        user->set(user, "password", "123456");
        user->set(user, "age", &age);
 
        table = object_new(orm->parent.allocator, "Test_User_Table", NULL);
        table->add_model(table, user);
        conn = orm->get_conn(orm);

        /* 清空表 */
        EXEC(conn->del(conn, "DELETE FROM test_user_table"));

        /* 新增*/
        EXEC(conn->insert_table(conn, table));
        table->reset(table); //释放table， 后面要用空的table去查询以验证。

        /* 查询 */
        conn->query_table(conn, table, "select * from test_user_table where mobile='13440129080';");
        count = table->count_model(table);
        THROW_IF(count != 1, -1);
        table->peek_at_model(table, 0, (void **)&test_user);
        THROW_IF(strcmp("user1", STR2A(test_user->nickname)) != 0, -1);
        table->reset(table); //释放table， 后面要用空的table去查询以验证。

        /* 修改 */
        user = object_new(orm->parent.allocator, "Test_User_Model", NULL);
        user->set(user, "nickname", "user1");
        user->set(user, "mobile", "13440129084");
        user->set(user, "password", "123456");
        user->set(user, "age", &age);
        table->add_model(table, user);
        EXEC(conn->insert_or_update_table(conn, table));
        table->reset(table); //释放table， 后面要用空的table去查询以验证。

        /* 验证是否修改成功 */
        conn->query_table(conn, table, "select * from test_user_table where mobile='13440129084';");
        count = table->count_model(table);
        THROW_IF(count != 1, -1);
        table->peek_at_model(table, 0, (void **)&test_user);
        THROW_IF(strcmp("user1", STR2A(test_user->nickname)) != 0, -1);
        table->reset(table); //释放table， 后面要用空的table去查询以验证。

        /* 删除 */
        EXEC(conn->del(conn, "DELETE FROM test_user_table"));
        conn->query_table(conn, table, "select * from test_user_table where mobile='13440129080';");
        count = table->count_model(table);
        THROW_IF(count != 0, -1);
    
        dbg_str(DBG_VIP, "command suc, func_name = %s,  file = %s, line = %d", 
                    __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {} FINALLY {
        orm->add_conn(orm, conn);
        object_destroy(table);
    }

    return ret;
}

static int __table_element_cmp(void *element, void *key) 
{
    Test_User_Model *user = (Test_User_Model *)element;
    Test_User_Model *search = (Test_User_Model *)key;

    if (strcmp(STR2A(user->nickname), STR2A(search->nickname)) == 0) {
        return 1;
    }

    return 0;
}

static int __test_orm_merge_table(Orm *orm)
{
    Orm_Conn *conn;
    Table *table;
    int i, expect_count = 1, ret = 1;
    Test_User_Model *user = NULL, *user2;
    int count = 22;
    char *json;

    TRY {
        user = object_new(orm->parent.allocator, "Test_User_Model", NULL);
        user2 = object_new(orm->parent.allocator, "Test_User_Model", NULL);
        table = object_new(orm->parent.allocator, "Test_User_Table", NULL);

        user->set(user, "nickname", "user");
        user->set(user, "mobile", "13440129081");
        user->set(user, "password", "123456");
        user->set(user, "count", &count);
        table->add_model(table, user);

        user2->set(user2, "nickname", "user");
        user2->set(user2, "mobile", "13440129082");
        user2->set(user2, "password", "123456");
        user2->set(user2, "count", &count);
        table->add_model(table, user2);

        EXEC(table->merge(table, __table_element_cmp));

        table->peek_at_model(table, 0, (void **)&user);
        THROW_IF(user->count != 44, -1);
        dbg_str(DBG_VIP, "command suc, func_name = %s,  file = %s, line = %d", 
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {
        json = table->to_json(table);
        dbg_str(DBG_ERROR, "json:%s", json);
    } FINALLY {
        object_destroy(table);
    }

    return ret;
}

/*
 * 测试 Orm_Conn->query 接口
 *
 * query 接口直接执行 SQL 并返回 JSON 字符串，
 * 不依赖 model 字段映射，因此可以处理 JOIN 查询的额外字段。
 *
 * 注意：仅使用测试专用表（test_user_table, test_order_table），
 * 不影响业务表。
 */
static int __test_orm_query(Orm *orm)
{
    Orm_Conn *conn;
    char json_buf[4096] = {0};
    int ret, bytes;

    TRY {
        conn = orm->get_conn(orm);
        THROW_IF(conn == NULL, -1);

        /* 创建测试订单表 */
        EXEC(conn->exec_sql(conn, "DROP TABLE IF EXISTS test_order_table"));
        EXEC(conn->exec_sql(conn, "CREATE TABLE test_order_table ("
                           "id INT AUTO_INCREMENT PRIMARY KEY, "
                           "productName VARCHAR(255), "
                           "price DECIMAL(10,2), "
                           "user_id INT)"));

        /* 清空并插入测试用户数据 */
        EXEC(conn->exec_sql(conn, "DELETE FROM test_user_table"));
        EXEC(conn->exec_sql(conn, "INSERT INTO test_user_table (nickname, mobile, password, age) "
                           "VALUES ('test_user_a', '13400000001', 'pwd1', 25), "
                           "('test_user_b', '13400000002', 'pwd2', 30)"));

        /* 插入测试订单数据 */
        EXEC(conn->exec_sql(conn, "INSERT INTO test_order_table (productName, price, user_id) "
                           "VALUES ('test_product_1', 99.99, 1), "
                           "('test_product_2', 199.99, 2), "
                           "('test_product_3', 299.99, 1)"));

        /* 1. 测试简单查询 */
        dbg_str(DBG_INFO, "test_orm_query: simple query");
        memset(json_buf, 0, sizeof(json_buf));
        bytes = conn->query(conn, json_buf, sizeof(json_buf),
                            "select id, nickname, mobile from test_user_table where nickname='%s'",
                            "test_user_a");
        THROW_IF(bytes <= 0, -1);
        dbg_str(DBG_INFO, "query result: %s", json_buf);
        THROW_IF(json_buf[0] != '[', -1);
        THROW_IF(strstr(json_buf, "nickname") == NULL, -1);
        THROW_IF(strstr(json_buf, "mobile") == NULL, -1);

        /* 2. 测试 JOIN 查询 - 这是 query_table 做不到的 */
        dbg_str(DBG_INFO, "test_orm_query: JOIN query");
        memset(json_buf, 0, sizeof(json_buf));
        bytes = conn->query(conn, json_buf, sizeof(json_buf),
                            "SELECT a.id, a.productName, a.price, "
                            "b.nickname, b.mobile "
                            "FROM test_order_table a "
                            "LEFT JOIN test_user_table b ON a.user_id = b.id "
                            "LIMIT 3");
        THROW_IF(bytes <= 0, -1);
        dbg_str(DBG_INFO, "JOIN query result: %s", json_buf);
        THROW_IF(json_buf[0] != '[', -1);
        /* 验证 JOIN 的额外字段存在 */
        THROW_IF(strstr(json_buf, "nickname") == NULL, -1);
        THROW_IF(strstr(json_buf, "mobile") == NULL, -1);
        /* 验证原表字段也存在 */
        THROW_IF(strstr(json_buf, "productName") == NULL, -1);

        /* 3. 测试空结果集 */
        dbg_str(DBG_INFO, "test_orm_query: empty result");
        memset(json_buf, 0, sizeof(json_buf));
        bytes = conn->query(conn, json_buf, sizeof(json_buf),
                            "SELECT * FROM test_order_table WHERE id = -1");
        THROW_IF(bytes <= 0, -1);
        dbg_str(DBG_INFO, "empty result: %s", json_buf);
        THROW_IF(strcmp(json_buf, "[]") != 0, -1);

        dbg_str(DBG_INFO, "command suc, func_name = %s,  file = %s, line = %d",
                __func__, extract_filename_from_path(__FILE__), __LINE__);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "test_orm_query failed, json_buf:%s", json_buf);
    } FINALLY {
        /* 清理测试订单表 */
        conn->exec_sql(conn, "DROP TABLE IF EXISTS test_order_table");
        orm->add_conn(orm, conn);
    }

    return ret;
}

static int test_orm_drop_test_table(Orm *orm)
{
    Orm_Conn *conn;
    int ret;

    conn = orm->get_conn(orm);
    if (conn) {
        ret = conn->drop_table(conn, "Test_User_Table");
    }
    orm->add_conn(orm, conn);


    return ret;
}

static int test_orm(TEST_ENTRY *entry)
{
    allocator_t *allocator = allocator_get_default_instance();
	Orm *orm; 
    int ret;
    
    TRY {
        dbg_str(DBG_VIP, "run orm");
        orm = object_new(allocator, "Orm", NULL);

        orm->set(orm, "host", "10.10.10.115");
        orm->set(orm, "user", "root");
        orm->set(orm, "password", "123456");
        orm->set(orm, "database_name", "test");
        EXEC(orm->run(orm));

        EXEC(__test_orm_create_table(orm));
        EXEC(__test_orm_model_operations(orm));
        EXEC(__test_orm_table_operations(orm));;
        EXEC(__test_orm_merge_table(orm));
        EXEC(__test_orm_query(orm));
    } CATCH (ret) {} FINALLY {
        test_orm_drop_test_table(orm);
        object_destroy(orm);
    }

    return ret;
}
REGISTER_TEST_CMD(test_orm);
