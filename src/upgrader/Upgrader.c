/**
 * @file Upgrader.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-05-23
 */

#include <libobject/upgrader/Upgrader.h>

/* 
 * 因为有些模块可能不需要node，但是需要upgrader， 所以单独一个
 * 模块，也可以单独使用upgrader命令直接升级，不用走node。这样
 * 也是node相对独立不需要依赖upgrader库， 如果需要可以动态加载。
 */

static int __construct(Upgrader *upgrader, char *init_str)
{
    return 0;
}

static int __deconstruct(Upgrader *upgrader)
{
    return 0;
}

static class_info_entry_t upgrader_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Upgrader, construct, __construct),
    Init_Nfunc_Entry(2, Upgrader, deconstruct, __deconstruct),
    Init_End___Entry(3, Upgrader),
};
REGISTER_CLASS("Upgrader", upgrader_class_info);