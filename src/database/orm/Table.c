/**
 * @file Table.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/database/orm/Table.h>
#include <libobject/database/orm/Model.h>
#include <libobject/core/String.h>

static int __construct(Table *table, char *init_str)
{
    Vector *models;
    Vector *meta_info;
    uint8_t trustee_flag = 1;
    int value_type = VALUE_TYPE_OBJ_POINTER;

    table->table_name = object_new(table->parent.allocator, "String", NULL);
    table->model_name = object_new(table->parent.allocator, "String", NULL);

    models = object_new(table->parent.allocator, "Vector", NULL);
    models->set(models, "/Vector/trustee_flag", &trustee_flag);
    models->set(models, "/Vector/value_type", &value_type);

    meta_info = object_new(table->parent.allocator, "Vector", NULL);
    meta_info->set(meta_info, "/Vector/trustee_flag", &trustee_flag);
    meta_info->set(meta_info, "/Vector/value_type", &value_type);

    table->models = models;
    table->meta_info = meta_info;

    return 0;
}

static int __deconstruct(Table *table)
{
    if (table->model_name) 
        object_destroy(table->model_name);

    if (table->table_name) 
        object_destroy(table->table_name);

    if (table->models) {
        object_destroy(table->models);
    }

    if (table->meta_info) {
        object_destroy(table->meta_info);
    }

    return 0;
}

int __add_model(Table *table, void *model)
{
    Vector *models = table->models;

    return models->add(models, model);
}

int __peek_at_model(Table *table, int index, void **model)
{
    Vector *models = table->models;
    int ret;

    ret = models->peek_at(models, index,  model);

    return ret;
}

int __count_model(Table *table)
{
    Vector *models = table->models;

    return models->count(models);
}

static int __add_field(Table *table,  void *value)
{
    Vector *meta_info;

    meta_info = table->meta_info;

    return meta_info->add(meta_info, value);
}

static Field *__get_field(Table *table, char *field_name)
{
    Vector *meta_info = table->meta_info;
    Field *field = NULL;
    int count, i;

    count = meta_info->count(meta_info);

    for (i = 0; i < count; i++) {
        meta_info->peek_at(meta_info, i, (void **)&field);
        if (field != NULL && field->name->equal(field->name, field_name)) {
            return field;
        }
    }

    return NULL;
}

static int __set_table_name(Table *table, char *table_name)
{
    table->table_name->assign(table->table_name, table_name);
    
    return 0;
}

static char * __get_table_name(Table *table)
{
    return table->table_name->get_cstr(table->table_name);
}

static int __set_model_name(Table *table, char *model_name)
{
    Vector *models = table->models;

    table->model_name->assign(table->model_name, model_name);
    models->set(models, "/Vector/class_name", model_name);
    
    return 0;
}

static char * __get_model_name(Table *table)
{
    return table->model_name->get_cstr(table->model_name);
}

static int __reset(Table *table)
{
    Vector *models = table->models;

    return models->reset(models);
}

static char * __to_json(Table *table)
{
    Vector *models = table->models;

    return models->to_json(models);
}

static int __assign(Table *table, char *json)
{
    Vector *models = table->models;
    char *model_name;
    int ret;

    TRY {
        model_name= table->get_model_name(table);
        EXEC(models->set(models, "/Vector/class_name", model_name));
        EXEC(models->assign(models, json));
    } CATCH (ret) {
    }

    return ret;
}

static int __for_each(Table *table, int (*func)(int index, void *element))
{
    Vector *models = table->models;

    models->for_each(models, func);

    return 1;
}

static int __for_each_arg(Table *table, int (*func)(int index, void *element, void *arg), void *arg)
{
    Vector *models = table->models;

    models->for_each_arg(models, func, arg);

    return 1;
}

static int __merge(Table *table, int (*cmp)(void *element, void *key))
{
    allocator_t *allocator = table->parent.allocator;
    Vector *models = table->models;
    Vector *new_models;
    char *model_name = NULL;
    uint8_t trustee_flag = 1;
    int value_type = VALUE_TYPE_OBJ_POINTER;
    int ret, index;
    Model *element = NULL, *search_model = NULL;

    TRY {
        model_name = table->get_model_name(table);
        new_models = object_new(allocator, "Vector", NULL);
        THROW_IF(new_models == NULL, -1);
        new_models->set(new_models, "/Vector/trustee_flag", &trustee_flag);
        new_models->set(new_models, "/Vector/value_type", &value_type);
        new_models->set(new_models, "/Vector/class_name", model_name);

        while ((index = models->get_end_index(models)) > 0) {
            EXEC(models->remove_back(models, &element));
            CONTINUE_IF(element == NULL);
            EXEC(new_models->search(new_models, cmp, element, &search_model, &index));
            if (search_model == NULL) {
                EXEC(element->cut(element));
                new_models->add_back(new_models, element);
                continue;
            }
            EXEC(search_model->add(search_model, element));
            search_model = NULL;
            object_destroy(element);
        }
        table->models = new_models;
    } CATCH (ret) {
        models = new_models;
    } FINALLY {
        object_destroy(models);
    }

    return ret;
}

static int __sort(Table *table, enum vector_sort_type_e type,  int (*func)(void *e1, void *e2))
{
    Vector *models = table->models;

    return models->sort(models, type, func);
}

static int __reset_from(Table *table, int index)
{
    Vector *models = table->models;

    return models->reset_from(models, index);
}

static int __filter(Table *table, int (*condition)(void *model, void *cond), void *cond, Table *out)
{
    Vector *models = table->models;

    return models->filter(models, condition, cond, out->models);
}

static int __add_table(Table *table, Table *src)
{
    Vector *models = table->models;

    return models->add_vector(models, src->models);
}

static int __copy_table(Table *table, Table *out)
{
    Vector *models = table->models;

    return models->copy(models, out->models);
}

static int __search(Table *table, int (*cmp)(void *element, void *key), void *key, void **out, int *index)
{
    Vector *models = table->models;

    return models->search(models, cmp, key, out, index);
}

static class_info_entry_t table_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Table, construct, __construct),
    Init_Nfunc_Entry(2 , Table, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Table, add_model, __add_model),
    Init_Vfunc_Entry(4 , Table, remove_model, NULL),
    Init_Vfunc_Entry(5 , Table, peek_at_model, __peek_at_model),
    Init_Vfunc_Entry(6 , Table, count_model, __count_model),
    Init_Vfunc_Entry(7 , Table, add_field, __add_field),
    Init_Vfunc_Entry(8 , Table, get_field, __get_field),
    Init_Vfunc_Entry(9 , Table, set_table_name, __set_table_name),
    Init_Vfunc_Entry(10, Table, get_table_name, __get_table_name),
    Init_Vfunc_Entry(11, Table, set_model_name, __set_model_name),
    Init_Vfunc_Entry(12, Table, get_model_name, __get_model_name),
    Init_Vfunc_Entry(13, Table, reset, __reset),
    Init_Vfunc_Entry(14, Table, to_json, __to_json),
    Init_Vfunc_Entry(15, Table, assign, __assign),
    Init_Vfunc_Entry(16, Table, for_each, __for_each),
    Init_Vfunc_Entry(17, Table, for_each_arg, __for_each_arg),
    Init_Vfunc_Entry(18, Table, merge, __merge),
    Init_Vfunc_Entry(19, Table, sort, __sort),
    Init_Vfunc_Entry(20, Table, reset_from, __reset_from),
    Init_Vfunc_Entry(21, Table, filter, __filter),
    Init_Vfunc_Entry(22, Table, add_table, __add_table),
    Init_Vfunc_Entry(23, Table, copy_table, __copy_table),
    Init_Vfunc_Entry(24, Table, search, __search),
    Init_End___Entry(25, Table),
};
REGISTER_CLASS(Table, table_class_info);

