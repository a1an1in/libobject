/**
 * @file FileTransfer.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2024-05-23
 */

#include "FileTransfer.h"

/* 
 * 因为有些模块可能不需要node，但是需要FileTransfer， 所以单独一个
 * 模块，也可以单独使用FileTransfer命令直接升级，不用走node。这样
 * 也是node相对独立不需要依赖FileTransfer库， 如果需要可以动态加载。
 */

static int __construct(FileTransfer *FileTransfer, char *init_str)
{
    return 0;
}

static int __deconstruct(FileTransfer *FileTransfer)
{
    return 0;
}

static class_info_entry_t file_transfer_class_info[] = {
    Init_Obj___Entry(0, Obj, parent),
    Init_Nfunc_Entry(1, FileTransfer, construct, __construct),
    Init_Nfunc_Entry(2, FileTransfer, deconstruct, __deconstruct),
    Init_End___Entry(3, FileTransfer),
};
REGISTER_CLASS(FileTransfer, file_transfer_class_info);