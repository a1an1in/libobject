/**
 * @file Object_Chain.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include <libobject/core/Object_Chain.h>
#include <libobject/core/String.h>

static int __construct(Object_Chain *chain, char *init_str)
{
    allocator_t *allocator = ((Obj *)chain)->allocator;
    uint8_t trustee_flag = 1;
    List *l;
    int value_type = VALUE_TYPE_OBJ_POINTER;

    l = object_new(allocator, "Linked_List", NULL);
    if (l == NULL) {
        return -1;
    }

    l->set(l, "/List/trustee_flag", &trustee_flag);
    l->set(l, "/List/value_type", &value_type);

    chain->list = l;

    return 0;
}

static int __deconstruct(Object_Chain *chain)
{
    object_destroy(chain->list);

    return 0;
}

static void * __new(Object_Chain *chain, char *class_name, char *data)
{ 
    allocator_t *allocator = ((Obj *)chain)->allocator;
    List *l = chain->list;
    Obj *o = NULL;
    int ret = 0;
    int assign_flag = 0;
    char *init_data;

    TRY {
        if (strcmp(class_name, "String") == 0 && data != NULL) {
            init_data = data;
            assign_flag = 1;
            data = NULL;
        }

        THROW_IF((o = object_new(allocator, class_name, data)) == NULL, -1);

        EXEC(l->add_back(l, o));

        if (assign_flag) {
            o->assign(o, init_data);
        }
    } CATCH {
        EXEC_IF(o != NULL, object_destroy(o));
        o = NULL;
    }
    ENDTRY;

    return o;
}

static class_info_entry_t object_chain_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Object_Chain, construct, __construct),
    Init_Nfunc_Entry(2, Object_Chain, deconstruct, __deconstruct),
    Init_Nfunc_Entry(3, Object_Chain, new, __new),
    Init_End___Entry(4, Object_Chain),
};
REGISTER_CLASS("Object_Chain", object_chain_class_info);

