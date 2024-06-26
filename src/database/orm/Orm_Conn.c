/**
 * @file Orm_Conn.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-07-25
 */

#include <libobject/database/orm/Orm_Conn.h>
#include <libobject/database/sql/Sql.h>
#include <libobject/core/Number.h>
#include <libobject/core/try.h>
#include <libobject/database/orm/Model.h>

static int __construct(Orm_Conn *conn, char *init_str)
{
    return 0;
}

static int __deconstruct(Orm_Conn *conn)
{
    if (conn->sql) {
        conn->close(conn);
        object_destroy(conn->sql);
    }

    return 0;
}

static int __open(Orm_Conn *conn, char *host, 
                  char *user, char *password, 
                  char *database_name)
{
    Sql *sql = NULL;
    int ret;

    TRY {
        sql = object_new(conn->parent.allocator, "Mysql", NULL);
        THROW_IF(sql == NULL, -1);

        sql->set(sql, "/Sql/host", host);
        sql->set(sql, "/Sql/user", user);
        sql->set(sql, "/Sql/password", password);
        sql->set(sql, "/Sql/database_name", database_name);

        EXEC(sql->connect(sql));
        conn->sql = sql;
    } CATCH (ret) {
        object_destroy(sql);
    }

    return ret;
}

static int __insert_model(Orm_Conn *conn, Model *model)
{
    Table *table;
    Sql *sql = conn->sql;
    Object_Cache *cache = sql->cache;
    void **member;
    String **str_p2 = NULL;
    String *type;
    Vector *meta_info;
    Field *field;
    int meta_count, i, str_len, ret = -1, set_flag = 0;
    char *column, *table_class_name, *table_name, *model_name;
    String *sql_statement, *sql_columns, *sql_values;
    class_info_entry_t * entry;

    TRY {
        table_class_name = model->get_table_class_name(model);

        sql_statement = cache->new(cache, "String", NULL);
        sql_columns   = cache->new(cache, "String", NULL);
        sql_values    = cache->new(cache, "String", NULL);
        table         = cache->new(cache, table_class_name, NULL);

        table_name = table->table_name->get_cstr(table->table_name);
        model_name = table->get_model_name(table);
        meta_info = table->meta_info;
        meta_count = meta_info->count(meta_info);
        sql_columns->assign(sql_columns, "(");
        sql_values->assign(sql_values, "(");

        dbg_str(DB_DETAIL, "meta_count:%d", meta_count);

        for (i = 0; i < meta_count; i++) {
            meta_info->peek_at(meta_info, i, (void **)&field);
            CONTINUE_IF(field == NULL);
            type = field->type;
            column = STR2A(field->name);
            member = model->get(model, column);
            THROW_IF(member == NULL, -1);
            CONTINUE_IF(*member == NULL);
            if (set_flag == 1) {
                sql_values->append(sql_values, ", ", -1);
            }

            THROW_IF((entry = object_get_entry_of_class(model_name, column)) == NULL, -1);
            if (entry->type <= ENTRY_TYPE_UN64 && entry->type >= ENTRY_TYPE_INT8_T) {
                int value;
                (*((Number **)member))->get_value((*((Number **)member)), NUMBER_TYPE_SIGNED_INT, &value);
                sql_values->format(sql_values, 1024, "%d", value);
                dbg_str(DB_DETAIL, "column:%s value:%d", column, value);
            } else if(entry->type == ENTRY_TYPE_STRING) {
                char *value;
                value = STR2A((*((String **)member)));
                if (strlen(value) == 0) continue;
                sql_values->format(sql_values, 1024, "'%s'", value);
                dbg_str(DB_DETAIL, "column:%s value:%s", column, value);
            } else if(entry->type == ENTRY_TYPE_VECTOR) {
                Vector *value;
                value = *((Vector **)member);
                CONTINUE_IF(value == NULL);
                sql_values->format(sql_values, 1024, "'%s'", value->to_json(value));
                dbg_str(DB_DETAIL, "column:%s value:%s", column, value->to_json(value));
            } else {
                dbg_str(DB_ERROR, "entry type:%d, model name:%s column_name_cstr :%s",
                        entry->type, ((Obj *)model)->name, column);
                THROW(-1);
            }

            if (!sql_columns->equal(sql_columns, "(")) {
                sql_columns->append(sql_columns, ", ", -1);
            }
            sql_columns->append(sql_columns, column, -1);
            set_flag = 1;
        }

        sql_columns->append(sql_columns, ")", -1);
        sql_values->append(sql_values, ")", -1);
        dbg_str(DB_DETAIL, "colums:%s values:%s", STR2A(sql_columns), STR2A(sql_values));

        THROW_IF(sql_columns->equal(sql_columns, "()"), -1);
        sql_statement->format(sql_statement, 1024, "INSERT INTO %s %s VALUES %s", 
                table_name, STR2A(sql_columns), STR2A(sql_values));

        dbg_str(DBG_DETAIL, "sql:%s", STR2A(sql_statement));
        EXEC(sql->insert(sql, STR2A(sql_statement)));
    } CATCH (ret) {
    } FINALLY {
        object_destroy(table);
        object_destroy(sql_statement);
        object_destroy(sql_columns);
        object_destroy(sql_values);
    }

    return ret;
}

static int __insert_table(Orm_Conn *conn, Table *table)
{
    int i;
    int count;
    Model *model;
    int ret;

    count = table->count_model(table);
    for (i = 0; i < count; i++) {
        table->peek_at_model(table, i, (void **)&model);
        if (model == NULL) {
            return -1;
        }
        ret = __insert_model(conn, model);
        if (ret < 0) {
            return ret;
        }
    }

    return 0;
}

static int __del(Orm_Conn *conn, char *sql_fmt, ...)
{
    Sql *sql = conn->sql;
    char sql_buffer[2048] = {0};
    va_list ap;
    int ret = 1;

    TRY {
        va_start(ap, sql_fmt);
        EXEC(vsnprintf(sql_buffer, 2048, sql_fmt, ap));
        va_end(ap);

        EXEC(sql->del(sql, sql_buffer));
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "del error, sql:%s", sql_buffer);
    }

    return ret;
}

static int __update_model(Orm_Conn *conn, Model *model)
{
    Table *table;
    Sql *sql = conn->sql;
    Object_Cache *cache = sql->cache;
    void **member;
    Vector *meta_info;
    int meta_count, i;
    Field *field;
    String *sql_statement, *sql_set, *sql_where;
    int str_len, ret = -1;
    char *table_class_name, *model_name;
    class_info_entry_t *entry;
    int set_flag = 0;

    TRY {
        table_class_name = model->get_table_class_name(model);

        sql_statement = cache->new(cache, "String", NULL);
        sql_set       = cache->new(cache, "String", NULL);
        sql_where     = cache->new(cache, "String", NULL);
        table         = cache->new(cache, table_class_name, NULL);
        model_name    = ((Obj *)model)->name;

        meta_info = table->meta_info;
        meta_count = meta_info->count(meta_info);
        dbg_str(DBG_DETAIL, "meta_count:%d", meta_count);

        for (i = 0; i < meta_count; i++) {
            CONTINUE_IF2(meta_info->peek_at(meta_info, i, (void **)&field), field == NULL);
            CONTINUE_IF2((member = model->get(model, STR2A(field->name))), *member == NULL);
            if (i == 0) {
                sql_where->format(sql_where, 1024, "%s=%d", STR2A(field->name), NUM2S32((*((Number **)member))));
                continue;
            }
            if (set_flag == 1) {
                sql_set->append(sql_set, ", ", -1);
            }

            THROW_IF((entry = object_get_entry_of_class(model_name, STR2A(field->name))) == NULL, -1);
            if (entry->type <= ENTRY_TYPE_UN64 && entry->type >= ENTRY_TYPE_INT8_T) {
                sql_set->format(sql_set, 1024, "%s=%d", STR2A(field->name), NUM2S32((*((Number **)member))));
            } else if(entry->type == ENTRY_TYPE_STRING) {
                sql_set->format(sql_set, 1024, "%s='%s'", STR2A(field->name), STR2A((*((String **)member))));
            } else if(entry->type == ENTRY_TYPE_VECTOR) {
                Vector *value = *((Vector **)member);
                CONTINUE_IF(value == NULL);
                sql_set->format(sql_set, 1024, "%s='%s'", STR2A(field->name), value->to_json(value));
            } else {
                THROW(-1);
            }

            set_flag = 1;
        }

        THROW_IF(sql_set->equal(sql_set, "") || sql_where->equal(sql_where, ""), -1);
        sql_statement->format(sql_statement, 1024, "UPDATE %s SET %s WHERE %s", 
                STR2A(table->table_name), STR2A(sql_set), STR2A(sql_where));

        dbg_str(DBG_DETAIL, "set clause:%s, where clause:%s", STR2A(sql_set), STR2A(sql_where));
        dbg_str(DBG_DETAIL, "sql:%s",STR2A(sql_statement));
        EXEC(sql->update(sql, STR2A(sql_statement)));
    } CATCH (ret) {
    } FINALLY {
        object_destroy(table);
        object_destroy(sql_statement);
        object_destroy(sql_set);
        object_destroy(sql_where);
    }

    return ret;
}

static int __update_table(Orm_Conn *conn, Table *table)
{
    int i;
    int count;
    Model *model;
    int ret = 1;

    TRY {
        count = table->count_model(table);
        for (i = 0; i < count; i++) {
            table->peek_at_model(table, i, (void **)&model);
            THROW_IF(model == NULL, -1);
            EXEC(__update_model(conn, model));
        }
    } CATCH (ret) {
    }

    return ret;
}

static int __update(Orm_Conn *conn, char *model_name, char *json)
{
    int ret;
    Vector *vector = NULL;
    Sql *sql = conn->sql;
    Object_Cache *cache = sql->cache;
    char init_data[1024] = {0};
    String *string;
    int count , i;
    Model *model = NULL;

    TRY {
        snprintf(init_data, 1024, "(%s):%s", model_name, json);
        vector = cache->new(cache, "Vector", init_data);
        count = vector->count(vector);
        dbg_str(DBG_DETAIL, "vector to json:%s", vector->to_json(vector));

        for (i = 0; i < count; i++) {
            vector->peek_at(vector, i, (void **)&model);
            THROW_IF(model == NULL, -1);

            EXEC(__update_model(conn, model));
            model = NULL;
        }
    } CATCH(ret) {
    } FINALLY {
        object_destroy(vector);
    }
    return ret;
}

static int __query_model(Orm_Conn *conn, Model *model, char *sql_fmt, ...)
{
    Sql *sql = conn->sql;
    Result *result;
    int i, row_count, column_count;
    String *column, *column_name;
    class_info_entry_t *entry;
    char *model_name;
    char sql_buffer[2048] = {0};
    va_list ap;
    Row *r;
    int ret = 1;

    TRY {
        va_start(ap, sql_fmt);
        EXEC(vsnprintf(sql_buffer, 2048, sql_fmt, ap));
        va_end(ap);

        EXEC(sql->query(sql, sql_buffer));
        result = sql->result;
        row_count = result->get_row_count(result);
        dbg_str(DBG_DETAIL, "query_model, row_count=%d", row_count);
        THROW_IF(row_count > 1 || row_count < 0, -1);
        THROW_IF(row_count == 0, 0);
        column_count = result->get_column_count(result);
        model_name = ((Obj *)model)->name;

        r = result->get_row(result, 0);
        for (i = 0; r != NULL && i < column_count; i++) {
            column_name = result->get_column(result, i);
            column = r->get_column(r, i);
            if (column == NULL || column_name == NULL) continue; 
            CONTINUE_IF(strlen(STR2A(column)) == 0 || strlen(STR2A(column_name)) == 0);

            THROW_IF((entry = object_get_entry_of_class(model_name, STR2A(column_name))) == NULL, -1);
            if (entry->type <= ENTRY_TYPE_UN64 && entry->type >= ENTRY_TYPE_INT8_T) {
                int v = strtol(STR2A(column), NULL, 10);
                model->set(model, STR2A(column_name), &v);
            } else if(entry->type == ENTRY_TYPE_STRING) {
                model->set(model, STR2A(column_name), STR2A(column));
            } else if(entry->type == ENTRY_TYPE_VECTOR) {
                model->set(model, STR2A(column_name), STR2A(column));
            } else {
                dbg_str(DB_ERROR, "entry type:%d, model name:%s column_name_cstr :%s column_value_cstr:%s",
                        entry->type, ((Obj *)model)->name, STR2A(column_name), STR2A(column));
                THROW(-1);
            }
        }
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "query model sql:%s", sql_buffer);
    }

    return ret;
}

static int __query_table(Orm_Conn *conn, Table *table, char *sql_fmt, ...)
{
    Sql *sql = conn->sql;
    allocator_t *allocator = conn->parent.allocator;
    Result *result;
    int i, j, row_count, column_count, ret, count = 0;
    String *column, *column_name, *type;
    char *model_name;
    class_info_entry_t *entry;
    Row *r;
    Vector *rows;
    Obj *model;
    Field *field = NULL;
    char sql_buffer[2048] = {0};
    va_list ap;

    TRY {
        va_start(ap, sql_fmt);
        EXEC(vsnprintf(sql_buffer, 2048, sql_fmt, ap));
        va_end(ap);

        ret = sql->query(sql, sql_buffer);
        result = sql->result;
        row_count = result->get_row_count(result);
        column_count = result->get_column_count(result);
        rows = result->rows;
        model_name = table->get_model_name(table);

        for (i = 0; i < row_count; i++) {
            r = result->get_row(result, i);
            model = object_new(allocator, model_name, NULL);
            for (j = 0; r != NULL && j < column_count; j++) {
                column_name = result->get_column(result, j);
                column = r->get_column(r, j);

                CONTINUE_IF(column == NULL || column_name == NULL);
                CONTINUE_IF(strlen(STR2A(column)) == 0 || strlen(STR2A(column_name)) == 0);

                /*
                 *field = table->get_field(table, STR2A(column_name));
                 *THROW_IF(field == NULL, -1);
                 *type = field->type;
                 */

                THROW_IF((entry = object_get_entry_of_class(model_name, STR2A(column_name))) == NULL, -1);
                if (entry->type <= ENTRY_TYPE_UN64 && entry->type >= ENTRY_TYPE_INT8_T) {
                    int v = strtol(STR2A(column), NULL, 10);
                    model->set(model, STR2A(column_name), &v);
                } else if(entry->type == ENTRY_TYPE_STRING) {
                    model->set(model, STR2A(column_name), STR2A(column));
                } else if(entry->type == ENTRY_TYPE_VECTOR) {
                    model->set(model, STR2A(column_name), STR2A(column));
                } else {
                    dbg_str(DB_ERROR, "entry type:%d, model name:%s column_name :%s column_value:%s",
                            entry->type, ((Obj *)model)->name, STR2A(column_name), STR2A(column));
                    THROW(-1);
                }
            }
            count++;
            EXEC(table->add_model(table, model));
        }
        THROW(count);
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "row_count:%d", row_count);
        dbg_str(DBG_ERROR, "query table sql:%s", sql_buffer);
    }

    return ret;
}

static int __close(Orm_Conn *conn)
{
    return conn->sql->close(conn->sql);
}

static int __create_table(Orm_Conn *conn, char *table_class_name)
{
    int ret;
    Table *table = NULL;
    allocator_t *allocator = conn->parent.allocator;
    Vector *meta_info;
    int meta_count, i, len;
    Field *field;
    String *sql_statement = NULL;
    char *table_name;

    TRY {
        dbg_str(DB_DETAIL, "conn create_table");
        table = object_new(allocator, table_class_name, NULL);
        THROW_IF(table == NULL, -1);
        meta_info = table->meta_info;
        meta_count = meta_info->count(meta_info);
        table_name = table->get_table_name(table);
        dbg_str(DB_DETAIL, "meta_count:%d", meta_count);
        dbg_str(DB_DETAIL, "table_name:%s", table_name);

        sql_statement = object_new(allocator, "String", NULL);
        sql_statement->format(sql_statement, 1024, "CREATE TABLE IF NOT EXISTS %s (", table_name);

        for (i = 0; i < meta_count; i++) {
            meta_info->peek_at(meta_info, i, (void **)&field);
            CONTINUE_IF(field == NULL);

            if (i == meta_count - 1) {
                sql_statement->format(sql_statement, 1024, "%s %s %s)",
                        STR2A(field->name), STR2A(field->type), STR2A(field->constraint));
            } else {
                sql_statement->format(sql_statement, 1024, "%s %s %s, ",
                        STR2A(field->name), STR2A(field->type), STR2A(field->constraint));
            }
        }

        dbg_str(DB_SUC, "create_table sql:%s", STR2A(sql_statement));
        EXEC(conn->sql->create_table(conn->sql, STR2A(sql_statement)));
    } CATCH (ret) {
        dbg_str(DB_ERROR, "create_table sql:%s", STR2A(sql_statement));
    } FINALLY {
        object_destroy(sql_statement);
        object_destroy(table);
    }

    return ret;
}

static int __drop_table(Orm_Conn *conn, char *table_class_name)
{
    allocator_t *allocator = conn->parent.allocator;
    Table *table;
    Sql *sql = conn->sql;
    char sql_statement[1024];
    char *table_name;
    int ret;

    table = object_new(allocator, table_class_name, NULL);
    table_name = table->get_table_name(table);
    sprintf(sql_statement, "DROP TABLE %s", table_name);

    ret = sql->drop_table(sql, sql_statement);
    object_destroy(table);

    return ret;
}

static int __insert_or_update_model(Orm_Conn *conn, Model *model)
{
    Sql *sql = conn->sql;
    Object_Cache *cache = sql->cache;
    String *type, *sql_statement, *sql_columns, *sql_values, *sql_update;
    Vector *meta_info;
    Field *field;
    int meta_count, i, ret = 1, set_flag = 0;
    char *column, *model_name, *table_class_name, *table_name;
    void **member = NULL;
    class_info_entry_t *entry;
    Table *table;

    TRY {
        table_class_name = model->get_table_class_name(model);

        sql_statement = cache->new(cache, "String", NULL);
        sql_columns   = cache->new(cache, "String", NULL);
        sql_values    = cache->new(cache, "String", NULL);
        sql_update    = cache->new(cache, "String", NULL);
        table         = cache->new(cache, table_class_name, NULL);

        table_name = STR2A(table->table_name);
        model_name = table->get_model_name(table);
        meta_info = table->meta_info;
        meta_count = meta_info->count(meta_info);
        sql_values->assign(sql_values, "(");
        sql_columns->assign(sql_columns, "(");

        for (i = 0; i < meta_count; i++) {
            meta_info->peek_at(meta_info, i, (void **)&field);
            CONTINUE_IF(field == NULL);
            type = field->type;
            column = STR2A(field->name);
            THROW_IF((member = model->get(model, column)) == NULL, -1);
            CONTINUE_IF(*member == NULL);
            if (set_flag == 1) {
                sql_values->append(sql_values, ", ", -1);
            }

            THROW_IF((entry = object_get_entry_of_class(model_name, column)) == NULL, -1);
            if (entry->type <= ENTRY_TYPE_UN64 && entry->type >= ENTRY_TYPE_INT8_T) {
                int value;
                (*((Number **)member))->get_value((*((Number **)member)), NUMBER_TYPE_SIGNED_INT, &value);
                sql_values->format(sql_values, 1024, "%d", value);
                dbg_str(DB_DETAIL, "column:%s value:%d", column, value);
            } else if(entry->type == ENTRY_TYPE_STRING) {
                char *value;
                value = STR2A((*((String **)member)));
                if (strlen(value) == 0) continue;
                sql_values->format(sql_values, 1024, "'%s'", value);
                dbg_str(DB_DETAIL, "column:%s value:%s", column, value);
            } else if(entry->type == ENTRY_TYPE_VECTOR) {
                Vector *value;
                value = *((Vector **)member);
                CONTINUE_IF(value == NULL);
                sql_values->format(sql_values, 1024, "'%s'", value->to_json(value));
                dbg_str(DB_DETAIL, "column:%s value:%s", column, value->to_json(value));
            } else {
                dbg_str(DB_ERROR, "entry type:%d, model name:%s column_name_cstr :%s",
                        entry->type, ((Obj *)model)->name, column);
                THROW(-1);
            }

            if (!sql_columns->equal(sql_columns, "(")) {
                sql_columns->append(sql_columns, ", ", -1);
                sql_update->append(sql_update, ", ", -1);
            }
            sql_columns->append(sql_columns, column, -1);
            sql_update->format(sql_update, 1024, "%s=VALUES(%s)", column, column);

            set_flag = 1;
        }

        sql_values->append(sql_values, ")", -1);
        sql_columns->append(sql_columns, ")", -1);
        sql_statement->format(sql_statement, 1024, "INSERT INTO %s %s VALUES %s ON DUPLICATE KEY UPDATE %s;", 
                table_name, STR2A(sql_columns), STR2A(sql_values), STR2A(sql_update));
        dbg_str(DBG_DETAIL, "insert_or_update_model sql:%s", STR2A(sql_statement));
        EXEC(sql->insert(sql, STR2A(sql_statement)));
    } CATCH (ret) {
    } FINALLY {
        object_destroy(sql_statement);
        object_destroy(sql_columns);
        object_destroy(sql_values);
        object_destroy(sql_update);
        object_destroy(table);
    }

    return ret;
}


static int __insert_or_update_table(Orm_Conn *conn, Table *table)
{
    int i;
    int count, end;
    Model *model = NULL;
    int ret = 1;

    TRY {
        count = table->count_model(table);
        end = table->models->get_end_index(table->models);
        SET_CATCH_INT_PARS(count, count);
        for (i = 0; i < end; i++) {
            table->peek_at_model(table, i, (void **)&model);
            CONTINUE_IF(model == NULL);
            EXEC(__insert_or_update_model(conn, model));
            model = NULL;
        }
    } CATCH (ret) {
        CATCH_SHOW_INT_PARS(DB_ERROR);
    }

    return ret;
}

static class_info_entry_t conn_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Orm_Conn, construct, __construct),
    Init_Nfunc_Entry(2 , Orm_Conn, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Orm_Conn, open, __open),
    Init_Vfunc_Entry(4 , Orm_Conn, create_table, __create_table),
    Init_Vfunc_Entry(5 , Orm_Conn, drop_table, __drop_table),
    Init_Vfunc_Entry(6 , Orm_Conn, insert_model, __insert_model),
    Init_Vfunc_Entry(7 , Orm_Conn, insert_table, __insert_table),
    Init_Vfunc_Entry(8 , Orm_Conn, del, __del),
    Init_Vfunc_Entry(9 , Orm_Conn, update_model, __update_model),
    Init_Vfunc_Entry(10, Orm_Conn, update_table, __update_table),
    Init_Vfunc_Entry(11, Orm_Conn, update, __update),
    Init_Vfunc_Entry(12, Orm_Conn, query_table, __query_table),
    Init_Vfunc_Entry(13, Orm_Conn, query_model, __query_model),
    Init_Vfunc_Entry(14, Orm_Conn, close, __close),
    Init_Vfunc_Entry(15, Orm_Conn, __insert_or_update_model, __insert_or_update_model),
    Init_Vfunc_Entry(16, Orm_Conn, insert_or_update_table, __insert_or_update_table),
    Init_End___Entry(17, Orm_Conn),
};
REGISTER_CLASS(Orm_Conn, conn_class_info);

