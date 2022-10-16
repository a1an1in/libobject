/**
 * @file Row.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/database/sql/Row.h>

static int __construct(Row *row, char *init_str)
{
    allocator_t *allocator  = row->parent.allocator;
    uint8_t trustee_flag = 1;
    int value_type = VALUE_TYPE_OBJ_POINTER;

    row->data = object_new(allocator, "Vector", NULL);
    row->data->set(row->data, "/Vector/trustee_flag", &trustee_flag);
    row->data->set(row->data, "/Vector/value_type", &value_type);

    return 0;
}

static int __deconstruct(Row *row)
{
    if (row->data != NULL) {
        object_destroy(row->data);
    }

    return 0;
}

static String * __get_column(Row *row, int index)
{
    String *ret;

    row->data->peek_at(row->data, index, (void **)&ret);

    return ret;
}

static int __set_column(Row *row, int index, String *value)
{

    row->data->add_at(row->data, index, value);

    return 1;
}

static int __get_column_count(Row *row)
{
    return row->data->count(row->data);
}

static int __reset(Row *row)
{
    row->data->reset(row->data);
}

static class_info_entry_t row_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Row, construct, __construct),
    Init_Nfunc_Entry(2, Row, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Row, get_column, __get_column),
    Init_Nfunc_Entry(4, Row, set_column, __set_column),
    Init_Nfunc_Entry(5, Row, get_column_count, __get_column_count),
    Init_Nfunc_Entry(6, Row, reset, __reset),
    Init_End___Entry(7, Row),
};
REGISTER_CLASS("Row", row_class_info);

