/**
 * @file File.c
 * @Synopsis  
 * @author alan lin
 * @version 
 * @date 2017-09-27
 */
/* Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */
#include <stdio.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/io/File.h>

static int __construct(File *file,char *init_str)
{
    return 0;
}

static int __deconstruct(File *file)
{
    dbg_str(EV_DETAIL,"file deconstruct,file addr:%p",file);

    return 0;
}

static int __open(File *file, char *path, char *mode)
{
    file->f = fopen(path, mode);

    if (file->f == NULL) {
        perror("fopen");
        return -1;
    }
}

int __read(File *file, void *dst, int len)
{
    return fread(dst, 1, len, file->f);
}

int __write(File *file, void *src, int len)
{
    return fwrite(src, 1, len, file->f);
}

static int __rename(File *file, char *path)
{
}

static int __close(File *file)
{
    return fclose(file->f);
}

static class_info_entry_t file_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , File, construct, __construct),
    Init_Nfunc_Entry(2 , File, deconstruct, __deconstruct),
    Init_Vfunc_Entry(3 , File, set, NULL),
    Init_Vfunc_Entry(4 , File, get, NULL),
    Init_Vfunc_Entry(5 , File, open, __open),
    Init_Vfunc_Entry(6 , File, read, __read),
    Init_Vfunc_Entry(7 , File, write, __write),
    Init_Vfunc_Entry(8 , File, rename, __rename),
    Init_Vfunc_Entry(9 , File, close, __close),
    Init_End___Entry(10, File),
};
REGISTER_CLASS("File", file_class_info);

int test_file()
{
    File *file;
    char *content = "hello world";
    allocator_t *allocator = allocator_get_default_alloc();

    file = OBJECT_NEW(allocator, File, NULL);

    file->open(file, "./test.txt", "w+");
    file->write(file, content, strlen(content));
    file->close(file);
    pause();
    object_destroy(file);

    return 1;
}
REGISTER_TEST_CMD(test_file);
