/**
 * @file Strategy.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2019-06-19
 */

#include "Strategy.h"

static int __construct(Strategy *strategy, char *init_str)
{
    return 0;
}

static int __deconstruct(Strategy *strategy)
{
    return 0;
}

static class_info_entry_t strategy_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Strategy, construct, __construct),
    Init_Nfunc_Entry(2, Strategy, deconstruct, __deconstruct),
    Init_End___Entry(3, Strategy),
};
REGISTER_CLASS("Strategy", strategy_class_info);

