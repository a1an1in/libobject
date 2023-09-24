/**
 * @file Gzip.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2023-09-21
 */
#include "Gzip.h"

static int __construct(Gzip *gzip, char *init_str)
{
    allocator_t *allocator = gzip->parent.parent.allocator;

    dbg_str(DBG_VIP, "run at here");
    gzip->c = object_new(allocator, "ZCompress", NULL);
    if (gzip->c == NULL) return -1;

    return 1;
}

static int __deconstruct(Gzip *gzip)
{
    object_destroy(gzip->c);
    return 1;
}

 static int __compress(Gzip *gzip, char *file_in, char *file_out)
 {
    Compress *c;
    int ret;

    TRY {
        dbg_str(DBG_VIP, "run at here");
        THROW_IF((c = gzip->c), -1);
        EXEC(c->deflate_file(c, file_in, file_out));
    } CATCH(ret) { } FINALLY {};

    return ret;
 }

 static int __uncompres(Gzip *gzip, char *file_in, char *file_out)
 {
    Compress *c;
    int ret;

    TRY {
        dbg_str(DBG_VIP, "run at here");
        THROW_IF((c = gzip->c), -1);
        EXEC(c->inflate_file(c, file_in, file_out));
    } CATCH(ret) { } FINALLY {};

    return ret;
 }

static class_info_entry_t gzip_class_info[] = {
    Init_Obj___Entry(0, Archive, parent),
    Init_Nfunc_Entry(1, Gzip, construct, __construct),
    Init_Nfunc_Entry(2, Gzip, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3, Gzip, compress, __compress),
    Init_Vfunc_Entry(4, Gzip, uncompress, __uncompres),
    Init_End___Entry(5, Gzip),
};
REGISTER_CLASS("Gzip", gzip_class_info);

