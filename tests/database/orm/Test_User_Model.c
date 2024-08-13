/**
 * @file Test_User_Model.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "Test_User_Model.h"

static int __construct(Test_User_Model *model, char *init_str)
{
    Model *m = (Model *)model;

    m->set_table_class_name(m, "Test_User_Table");

    return 0;
}

static int __deconstruct(Test_User_Model *model)
{
    object_destroy(model->id);
    object_destroy(model->nickname);
    object_destroy(model->mobile);
    object_destroy(model->password);
    object_destroy(model->age);
    object_destroy(model->signature);
    object_destroy(model->portrait_url);
    object_destroy(model->email);
    object_destroy(model->count);

    return 0;
}

static int __add(Test_User_Model *dst, Test_User_Model *src) 
{
    int count = NUM2S32(dst->count) + NUM2S32(src->count);
    dst->set(dst, "count", &count);
    return 0;
}

static class_info_entry_t user_model_class_info[] = {
    Init_Obj___Entry(0 , Model, parent),
    Init_Nfunc_Entry(1 , Test_User_Model, construct, __construct),
    Init_Nfunc_Entry(2 , Test_User_Model, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , Test_User_Model, set, NULL),
    Init_Vfunc_Entry(4 , Test_User_Model, get, NULL),
    Init_Vfunc_Entry(5 , Test_User_Model, to_json, NULL),
    Init_Vfunc_Entry(6 , Test_User_Model, add, __add),
    Init_SN32__Entry(7 , Test_User_Model, id, NULL),
    Init_Str___Entry(8 , Test_User_Model, nickname, NULL),
    Init_Str___Entry(9 , Test_User_Model, mobile, NULL),
    Init_Str___Entry(10, Test_User_Model, password, NULL),
    Init_Str___Entry(11, Test_User_Model, signature, NULL),
    Init_Str___Entry(12, Test_User_Model, portrait_url, NULL),
    Init_Str___Entry(13, Test_User_Model, email, NULL),
    Init_SN32__Entry(14, Test_User_Model, age, NULL),
    Init_SN32__Entry(15, Test_User_Model, count, NULL),
    Init_Str___Entry(16, Test_User_Model, verification_code, NULL),
    Init_End___Entry(17, Test_User_Model),
};
REGISTER_CLASS(Test_User_Model, user_model_class_info);

