/**
 * @file Result.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/database/sql/Result.h>

static int __construct(Result *result, char *init_str)
{
    allocator_t *allocator  = result->parent.allocator;
    uint8_t trustee_flag = 1;
    int value_type = VALUE_TYPE_OBJ_POINTER;

    result->rows = object_new(allocator, "Vector", NULL);
    result->rows->set(result->rows, "/Vector/trustee_flag", &trustee_flag);
    result->rows->set(result->rows, "/Vector/value_type", &value_type);

    result->columns = object_new(allocator, "Vector", NULL);
    result->columns->set(result->columns, "/Vector/trustee_flag", &trustee_flag);
    result->columns->set(result->columns, "/Vector/value_type", &value_type);

    return 0;
}

static int __deconstruct(Result *result)
{
    object_destroy(result->rows);
    object_destroy(result->columns);

    return 0;
}

static Row *__get_row(Result *result, int index)
{
    Vector *rows = result->rows;
    Row *ret = NULL;

    rows->peek_at(rows, index, (void **)&ret);

    return ret;
}

static int __set_row(Result *result, int index, Row *row)
{
    Vector *rows = result->rows;

    return rows->add_at(rows, index, row);
}

static int __set_row_at_back(Result *result, Row *row)
{
    Vector *rows = result->rows;

    return rows->add_back(rows, row);
}

static String *__get_column(Result *result, int index)
{
    Vector *columns = result->columns;
    String *ret = NULL;

    columns->peek_at(columns, index, (void **)&ret);

    return ret;
}

static int __set_column(Result *result, int index, String *data)
{
    Vector *columns = result->columns;

    return columns->add_at(columns, index, data);
}

int __get_row_count(Result *result)
{
    return result->rows->count(result->rows);
}

int __get_column_count(Result *result)
{
    Vector *columns = result->columns;

    return columns->count(columns);
}

int __reset(Result *result)
{
    result->rows->reset(result->rows);
    result->columns->reset(result->columns);

    return 0;
}
static class_info_entry_t result_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Result, construct, __construct),
    Init_Nfunc_Entry(2 , Result, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Result, get_row, __get_row),
    Init_Vfunc_Entry(4 , Result, set_row, __set_row),
    Init_Vfunc_Entry(5 , Result, set_row_at_back, __set_row_at_back),
    Init_Vfunc_Entry(6 , Result, get_column, __get_column),
    Init_Vfunc_Entry(7 , Result, set_column, __set_column),
    Init_Vfunc_Entry(8 , Result, get_row_count, __get_row_count),
    Init_Vfunc_Entry(9 , Result, get_column_count, __get_column_count),
    Init_Vfunc_Entry(10, Result, reset, __reset),
    Init_End___Entry(11, Result),
};
REGISTER_CLASS(Result, result_class_info);

