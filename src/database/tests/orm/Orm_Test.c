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
#include <libobject/core/utils/registry/registry.h>
#include "Orm_Test.h"
#include "Test_User_Model.h"

static int __construct(Orm_Test *test, char *init_str)
{
    Orm *orm; 

    orm = object_new(test->parent.obj.allocator, "Orm", NULL);

    orm->set(orm, "host", "127.0.0.1");
    orm->set(orm, "user", "root");
    orm->set(orm, "password", "123456");
    orm->set(orm, "database_name", "test");

    orm->run(orm);

    test->orm = orm;

    return 0;
}

static int __deconstruct(Orm_Test *test)
{
    Orm *orm = test->orm; 

    if (test->orm)
        object_destroy(test->orm);

    return 0;
}

static int __setup(Orm_Test *test, char *init_str)
{
    return 1;
}

static int __teardown(Orm_Test *test)
{
    return 1;
}

static int __test_create(Orm_Test *test)
{
    Orm *orm = test->orm; 
    Orm_Conn *conn;
    int ret;

    conn = orm->get_conn(orm);
    if (conn) {
        ret = conn->create_table(conn, "Test_User_Table"); /* 专用于测试创建表 */
    }
    orm->add_conn(orm, conn);

    return ret;
}

static int __test_insert_table(Orm_Test *test)
{
    Orm *orm = test->orm; 
    Orm_Conn *conn;
    Test_User_Model *user;
    Table *table;
    int age = 22;

    dbg_str(DBG_DETAIL, "test_insert_table");
    user = object_new(orm->parent.allocator, "Test_User_Model", NULL);
    user->set(user, "nickname", "user1");
    user->set(user, "mobile", "13440129080");
    user->set(user, "password", "123456");
    user->set(user, "age", &age);

    table = object_new(orm->parent.allocator, "Test_User_Table", NULL);
    table->add_model(table, user);

    conn = orm->get_conn(orm);
    if (conn) {
        conn->insert_table(conn, table);
    }
    orm->add_conn(orm, conn);

    object_destroy(table);

    return 1;
}

static int __test_del(Orm_Test *test)
{
    Orm *orm = test->orm; 
    Orm_Conn *conn;
    int ret = 1;

    conn = orm->get_conn(orm);
    if (conn) {
        ret = conn->del(conn, "DELETE FROM test_user_table");
    }
    orm->add_conn(orm, conn);

    dbg_str(DBG_DETAIL, "ret = %d", ret);

    return ret;
}

static int __test_update_model(Orm_Test *test)
{
    Orm *orm = test->orm;
    Orm_Conn *conn;
    Test_User_Model *user;
    Test_User_Model *test_user;
    allocator_t * allocator = orm->parent.allocator;
    Table *table;
    int age = 22;
    int id = 6;
    int ret = 1;
    int count;
    
    TRY {
        dbg_str(DBG_DETAIL, "test_insert_table");
        table = object_new(allocator, "Test_User_Table", NULL);
        user = object_new(allocator, "Test_User_Model", NULL);
        conn = orm->get_conn(orm);
        EXEC(conn->del(conn, "DELETE FROM test_user_table"));

        /* add */
        user->set(user, "nickname", "user1");
        user->set(user, "mobile", "13440129082");
        user->set(user, "password", "123456");
        user->set(user, "age", &age);
        EXEC(conn->insert_model(conn, (Model *)user));

        /* query */
        conn->query_table(conn, table, "select * from test_user_table where mobile='13440129082';");
        count = table->count_model(table);
        THROW_IF(count < 1, -1);
        table->peek_at_model(table, 0, (void **)&test_user);
        dbg_str(DBG_DETAIL, "user:%s", test_user->to_json(test_user));
        id = NUM2S32(test_user->id);
        dbg_str(DBG_DETAIL, "update user Id:%d", id);

        /* update */
        user->set(user, "id", &id);
        user->set(user, "nickname", "user1");
        user->set(user, "mobile", "15440129083");
        user->set(user, "password", "123456");
        user->set(user, "age", &age);
        EXEC(conn->update_model(conn, (Model *)user));

        /* query */
        table->reset(table);
        conn->query_table(conn, table, "select * from test_user_table where mobile='15440129083';");
        count = table->count_model(table);
        dbg_str(DBG_DETAIL, "query row count:%d", count);
        THROW_IF(count != 1, -1);
        table->peek_at_model(table, 0, (void **)&test_user);
        dbg_str(DBG_DETAIL, "user:%s", test_user->to_json(test_user));
        THROW_IF(strcmp(STR2A(test_user->mobile), "15440129083") != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    } FINALLY {
        object_destroy(user);
        object_destroy(table);
        orm->add_conn(orm, conn);
    }

    return ret;
}

static int __test_update_json(Orm_Test *test)
{
    Orm *orm = test->orm;
    Orm_Conn *conn;
    Test_User_Model *user;
    Test_User_Model *test_user;
    allocator_t * allocator = orm->parent.allocator;
    Table *table;
    int age = 22;
    int id = 6;
    int ret = 1;
    int count;
    
    TRY {
        dbg_str(DBG_DETAIL, "test_insert_table");
        table = object_new(allocator, "Test_User_Table", NULL);
        user = object_new(allocator, "Test_User_Model", NULL);
        conn = orm->get_conn(orm);
        ret = conn->del(conn, "DELETE FROM test_user_table");

        /* add */
        user->set(user, "nickname", "user1");
        user->set(user, "mobile", "13440129082");
        user->set(user, "password", "123456");
        user->set(user, "age", &age);
        dbg_str(DBG_DETAIL, "user:%s", user->to_json(user));
        EXEC(conn->insert_model(conn, (Model *)user));

        /* query */
        conn->query_table(conn, table, "select * from test_user_table where mobile='13440129082';");
        count = table->count_model(table);
        THROW_IF(count < 1, -1);
        table->peek_at_model(table, 0, (void **)&test_user);
        dbg_str(DBG_DETAIL, "user:%s", test_user->to_json(test_user));
        id = NUM2S32(test_user->id);
        dbg_str(DBG_DETAIL, "update user Id:%d", id);

        /* update */
        user->set(user, "id", &id);
        user->set(user, "nickname", "user1");
        user->set(user, "mobile", "15440129083");
        user->set(user, "password", "123456");
        user->set(user, "age", &age);
        EXEC(conn->update(conn, "Test_User_Model", user->to_json(user)));
        dbg_str(DBG_DETAIL, "user:%s", user->to_json(user));

        /* query */
        table->reset(table);
        conn->query_table(conn, table, "select * from test_user_table where mobile='15440129083';");
        count = table->count_model(table);
        dbg_str(DBG_DETAIL, "query row count:%d", count);
        THROW_IF(count != 1, -1);
        table->peek_at_model(table, 0, (void **)&test_user);
        dbg_str(DBG_DETAIL, "user:%s", test_user->to_json(test_user));
        THROW_IF(strcmp(STR2A(test_user->mobile), "15440129083") != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    } FINALLY {
        object_destroy(user);
        object_destroy(table);
        orm->add_conn(orm, conn);
    }

    return ret;
}

static int __test_query_table(Orm_Test *test)
{
    Orm *orm = test->orm; 
    Orm_Conn *conn;
    Table *table;
    int count, i, expect_count = 1, ret = 1;
    Test_User_Model *user;
    int age = 22;

    TRY {
        conn = orm->get_conn(orm);
        if (conn == NULL) return 0;
        THROW_IF(conn == NULL, -1);
        EXEC(conn->del(conn, "DELETE FROM test_user_table"));

        user = object_new(orm->parent.allocator, "Test_User_Model", NULL);
        table = object_new(orm->parent.allocator, "Test_User_Table", NULL);

        user->set(user, "nickname", "user2");
        user->set(user, "mobile", "13440129081");
        user->set(user, "password", "123456");
        user->set(user, "age", &age);

        conn->insert_model(conn, (Model *)user);
        conn->query_table(conn, table, "select * from test_user_table;");
        count = table->count_model(table);

        object_destroy(user);
        dbg_str(DBG_DETAIL, "model count=%d", count);
        for (i = 0; i < count; i++) {
            table->peek_at_model(table, i, (void **)&user);
            dbg_str(DBG_DETAIL, "user:%s", user->to_json(user));
            user = NULL;
        }

        THROW_IF(count != expect_count, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    } FINALLY {
        object_destroy(table);
        orm->add_conn(orm, conn);
    }

    return ret;
}

static int __test_query_model(Orm_Test *test)
{
    Orm *orm = test->orm; 
    Orm_Conn *conn;
    int count, i, expect_count = 1, ret = 1;
    Test_User_Model *user = NULL;
    int age = 22;

    TRY {
        conn = orm->get_conn(orm);
        THROW_IF(conn == NULL, -1);
        EXEC(conn->del(conn, "DELETE FROM test_user_table"));
        user = object_new(orm->parent.allocator, "Test_User_Model", NULL);

        user->set(user, "nickname", "user2");
        user->set(user, "mobile", "13440129081");
        user->set(user, "password", "123456");
        user->set(user, "age", &age);
        conn->insert_model(conn, (Model *)user);

        user->set(user, "nickname", "user3");
        user->set(user, "mobile", "13440129082");
        user->set(user, "password", "123456");
        user->set(user, "age", &age);
        conn->insert_model(conn, (Model *)user);

        conn->query_model(conn, (Model *)user, "select * from test_user_table where mobile=%s;", "13440129081");
        THROW_IF(strcmp(STR2A(user->nickname), "user2") != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        dbg_str(DBG_ERROR, "user:%s", user->to_json(user));
    } FINALLY {
        if (user != NULL) {
            object_destroy(user);
        }
        orm->add_conn(orm, conn);
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

static int __test_merge_table(Orm_Test *test)
{
    Orm *orm = test->orm; 
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

        table->peek_at_model(table, 0, &user);
        THROW_IF(NUM2S32(user->count) != 44, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
        json = table->to_json(table);
        dbg_str(DBG_ERROR, "json:%s", json);
    } FINALLY {
        object_destroy(table);
    }

    return ret;
}

static int __test_insert_or_update_table(Orm_Test *test)
{
    Orm *orm = test->orm; 
    Orm_Conn *conn;
    int count, i, expect_count = 1, ret = 1, id = 0;
    Test_User_Model *user = NULL, *user2;
    Table *table;
    int age = 22;

    TRY {
        conn = orm->get_conn(orm);
        THROW_IF(conn == NULL, -1);
        EXEC(conn->del(conn, "DELETE FROM test_user_table"));
        user  = object_new(orm->parent.allocator, "Test_User_Model", NULL);
        user2 = object_new(orm->parent.allocator, "Test_User_Model", NULL);
        table = object_new(orm->parent.allocator, "Test_User_Table", NULL);

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

        user->set(user, "nickname", "user1");
        user->set(user, "mobile", "13440129083");
        user->set(user, "password", "123456");
        user->set(user, "age", &age);
        user->set(user, "id", &id);
        table->add_model(table, user);

        user2->set(user2, "nickname", "user2");
        user2->set(user2, "mobile", "13440129084");
        user2->set(user2, "password", "123456");
        user2->set(user2, "age", &age);
        table->add_model(table, user2);
        EXEC(conn->insert_or_update_table(conn, table));

        EXEC(conn->query_model(conn, (Model *)user, "select * from test_user_table where nickname='%s';", "user1"));
        dbg_str(DBG_DETAIL, "json:%s", user->to_json(user));
        THROW_IF(strcmp(STR2A(user->mobile), "13440129083") != 0, -1);
    } CATCH (ret) {
        TEST_SET_RESULT(test, ERROR_FUNC(), ERROR_LINE(), ERROR_CODE());
    } FINALLY {
        object_destroy(table);
        orm->add_conn(orm, conn);
    }

    return ret;
}

static int __test_drop(Orm_Test *test)
{
    Orm *orm = test->orm; 
    Orm_Conn *conn;
    int ret;

    conn = orm->get_conn(orm);
    if (conn) {
        ret = conn->drop_table(conn, "Test_User_Table");
    }
    orm->add_conn(orm, conn);

    return ret;
}

static class_info_entry_t orm_test_class_info[] = {
    Init_Obj___Entry(0 , Test, parent),
    Init_Nfunc_Entry(1 , Orm_Test, construct, __construct),
    Init_Nfunc_Entry(2 , Orm_Test, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Orm_Test, set, NULL),
    Init_Vfunc_Entry(4 , Orm_Test, get, NULL),
    Init_Vfunc_Entry(5 , Orm_Test, setup, __setup),
    Init_Vfunc_Entry(6 , Orm_Test, teardown, __teardown),
    Init_Vfunc_Entry(7 , Orm_Test, test_create, __test_create),
    Init_Vfunc_Entry(8 , Orm_Test, test_insert_table, __test_insert_table),
    Init_Vfunc_Entry(9 , Orm_Test, test_del, __test_del),
    Init_Vfunc_Entry(10, Orm_Test, test_update_model, __test_update_model),
    Init_Vfunc_Entry(11, Orm_Test, test_update_json, __test_update_json),
    Init_Vfunc_Entry(12, Orm_Test, test_query_table, __test_query_table),
    Init_Vfunc_Entry(13, Orm_Test, test_query_model, __test_query_model),
    Init_Vfunc_Entry(14, Orm_Test, test_merge_table, __test_merge_table),
    Init_Vfunc_Entry(15, Orm_Test, test_insert_or_update_table, __test_insert_or_update_table),
    Init_Vfunc_Entry(16, Orm_Test, test_drop, __test_drop),
    Init_End___Entry(17, Orm_Test),
};
REGISTER_CLASS("Orm_Test", orm_test_class_info);
