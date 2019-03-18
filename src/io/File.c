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
#include <libobject/core/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/event/event_base.h>
#include <libobject/io/file.h>

static int __construct(File *file,char *init_str)
{
    allocator_t *allocator = file->obj.allocator;
    configurator_t * c;
    char buf[2048];

    dbg_str(EV_DETAIL,"file construct, file addr:%p",file);


    return 0;
}

static int __deconstrcut(File *file)
{
    dbg_str(EV_DETAIL,"file deconstruct,file addr:%p",file);

    return 0;
}

static int __set(File *file, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        file->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        file->get = value;
    }
    else if (strcmp(attrib, "construct") == 0) {
        file->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        file->deconstruct = value;
    } else {
        dbg_str(EV_DETAIL,"file set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(File *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING,"file get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t file_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , File, construct, __construct),
    Init_Nfunc_Entry(2 , File, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , File, set, NULL),
    Init_Vfunc_Entry(4 , File, get, NULL),
    Init_End___Entry(5 ),
};
REGISTER_CLASS("File",file_class_info);

void test_obj_file()
{
    File *file;
    allocator_t *allocator = allocator_get_default_alloc();
    configurator_t * c;
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    c = cfg_alloc(allocator); 
    dbg_str(EV_SUC, "configurator_t addr:%p",c);
    cfg_config(c, "/File", CJSON_STRING, "name", "alan file") ;  

    file = OBJECT_NEW(allocator, File,c->buf);

    object_dump(file, "File", buf, 2048);
    dbg_str(EV_DETAIL,"File dump: %s",buf);

    object_destroy(file);
    cfg_destroy(c);
}
