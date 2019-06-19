/**
 * @file Application.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/argument/Application.h>

static int __construct(Application *app, char *init_str)
{
    return 0;
}

static int __deconstruct(Application *app)
{
    return 0;
}

static class_info_entry_t application_class_info[] = {
    Init_Obj___Entry(0, Command, parent),
    Init_Nfunc_Entry(1, Application, construct, __construct),
    Init_Nfunc_Entry(2, Application, deconstruct, __deconstruct),
    Init_End___Entry(3, Application),
};
REGISTER_CLASS("Application", application_class_info);

