/**
 * @file Model.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/database/orm/Model.h>
#include <libobject/core/policy.h>
#include <libobject/core/String.h>
#include <libobject/core/Vector.h>

static int __construct(Model *model, char *init_str)
{
    model->table_class_name = object_new(model->parent.allocator, "String", NULL);

    model->table_class_name->assign(model->table_class_name, "User_Table");

    return 0;
}

static int __deconstruct(Model *model)
{
    if (model->table_class_name)
        object_destroy(model->table_class_name);

    return 0;
}

int __set_table_class_name(Model *model, char *table_class_name)
{
    model->table_class_name->assign(model->table_class_name, table_class_name);
}

char *__get_table_class_name(Model *model)
{
    return model->table_class_name->get_cstr(model->table_class_name);
}

static int __to_json__(void *obj, char *type_name, cjson_t *object) 
{
    class_deamon_t *deamon;
    class_info_entry_t *entry;
    void *(*get)(void *obj, char *attrib) = NULL;
    int i;
    cjson_t *item;
    void *value;
    Obj *o = (Obj *)obj;
    int ret;

    TRY {
        deamon = class_deamon_get_global_class_deamon();
        entry  = (class_info_entry_t *) 
                 class_deamon_search_class(deamon, (char *)type_name);

        get = __object_get_ancestor_class_func(entry, (char *)"get");
        THROW_IF(get == NULL, -1);

        for (i = 0; entry[i].type != ENTRY_TYPE_END; i++) {
            SET_CATCH_STR_PARS(type_name, entry[i].value_name);
            THROW_IF(entry[i].type > ENTRY_TYPE_MAX_TYPE, -1);
            if (entry[i].type == ENTRY_TYPE_OBJ) {
                CONTINUE_IF(strcmp(entry[i].type_name, "Obj") == 0);
                CONTINUE_IF(strcmp(entry[i].type_name, "Model") == 0);
                item = cjson_create_object();
                cjson_add_item_to_object(object, entry[i].type_name, item);
                dbg_str(DBG_DETAIL, "%s to json", entry[i].type_name);
                __to_json__(obj, entry[i].type_name, item);
            } else {
                CONTINUE_IF(g_obj_to_json_policy[entry[i].type].policy == NULL);
                strcpy(o->target_name, type_name);

                value = get(obj, entry[i].value_name);
                CONTINUE_IF(value == NULL);
                EXEC(g_obj_to_json_policy[entry[i].type].policy(object, entry[i].value_name, value));
            }
        }   
    } CATCH (ret) {
        dbg_str(DBG_ERROR, "%s to_json error, par1=%s, par2=%s", type_name, ERROR_PTR_PAR1(), ERROR_PTR_PAR2());
    }

    return ret;
}

static char *__to_json(Obj *obj) 
{
    String *json = (String *)obj->json;
    cjson_t *root;
    char *out;
    int len;

    if (json == NULL) {
        json = object_new(obj->allocator, "String", NULL);
        obj->json = json;
    } else {
        json->reset(json);
    }

    root = cjson_create_object();

    __to_json__(obj, obj->name, root);

    out = cjson_print(root);
    json->assign(json, out);
    cjson_delete(root);
    free(out);

    return json->get_cstr(json);
}

static char *__add(Model *dst, Model *src) 
{
    return 0;
}

static char *__cut(Model *model) 
{
    return 0;
}

static class_info_entry_t user_model_class_info[] = {
    Init_Obj___Entry(0 , Obj, parent),
    Init_Nfunc_Entry(1 , Model, construct, __construct),
    Init_Nfunc_Entry(2 , Model, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Model, set, NULL),
    Init_Vfunc_Entry(4 , Model, get, NULL),
    Init_Vfunc_Entry(5 , Model, to_json, __to_json),
    Init_Vfunc_Entry(6 , Model, set_table_class_name, __set_table_class_name),
    Init_Vfunc_Entry(7 , Model, get_table_class_name, __get_table_class_name),
    Init_Vfunc_Entry(8 , Model, add, __add),
    Init_Vfunc_Entry(9 , Model, cut, __cut),
    Init_Str___Entry(10, Model, table_class_name, NULL),
    Init_End___Entry(11, Model),
};
REGISTER_CLASS(Model, user_model_class_info);
