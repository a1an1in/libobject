/**
 * @file Explorer.c
 * @Synopsis 
 *  explorer 主要提供访问设备, 文件上传和下载服务，应用程序升级服务。 
 * @author alan lin
 * @version 
 * @date 2024-01-26
 */

#include "Explorer.h"

static int __construct(Explorer *Explorer, char *init_str)
{
    return 0;
}

static int __deconstruct(Explorer *Explorer)
{
    return 0;
}

static class_info_entry_t Explorer_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, Explorer, construct, __construct),
    Init_Nfunc_Entry(2, Explorer, deconstruct, __deconstruct),
    Init_End___Entry(3, Explorer),
};
REGISTER_CLASS("Explorer", Explorer_class_info);