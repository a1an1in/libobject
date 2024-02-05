/**
 * @file Transceiver.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "Transceiver.h"

static int __construct(Transceiver *trans, char *init_str)
{
    return 0;
}

static int __deconstruct(Transceiver *trans)
{
    return 0;
}

static class_info_entry_t transceiver_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Transceiver, construct, __construct),
    Init_Nfunc_Entry(2, Transceiver, deconstruct, __deconstruct),
    Init_End___Entry(3, Transceiver),
};
REGISTER_CLASS("Transceiver", transceiver_class_info);