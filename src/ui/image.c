/**
 * @file image.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2016-12-03
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
#include <libdbg/debug.h>
#include <libobject/ui/image.h>

static int __construct(Image *image,char *init_str)
{
    dbg_str(OBJ_DETAIL,"image construct, image addr:%p",image);
    image->path = (String *)OBJECT_NEW(((Obj *)image)->allocator, String,NULL);
    if (image->path == NULL) {
        dbg_str(DBG_ERROR,"construct image error");
        return -1;
    }

    return 0;
}

static int __deconstrcut(Image *image)
{
    dbg_str(OBJ_DETAIL,"image deconstruct,image addr:%p",image);
    object_destroy(image->path);

    return 0;
}

static int __set(Image *image, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        image->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        image->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        image->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        image->deconstruct = value;
    } else if (strcmp(attrib, "load_image") == 0) {
        image->load_image = value;
    } else {
        dbg_str(OBJ_WARNNING,"image set,  \"%s\" setting is not support",attrib);
    }

    return 0;
}

static void * __get(Image *image, char *attrib)
{
    if (strcmp(attrib, "x") == 0){ 
    } else {
        dbg_str(OBJ_WARNNING,"image get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static int __load_image(Image *image)
{
    dbg_str(DBG_SUC,"SDL_Graph load image");
}

static class_info_entry_t image_class_info[] = {
    [0 ] = {ENTRY_TYPE_OBJ,"Obj","obj",NULL,sizeof(void *)},
    [1 ] = {ENTRY_TYPE_FUNC_POINTER,"","set",__set,sizeof(void *)},
    [2 ] = {ENTRY_TYPE_FUNC_POINTER,"","get",__get,sizeof(void *)},
    [3 ] = {ENTRY_TYPE_FUNC_POINTER,"","construct",__construct,sizeof(void *)},
    [4 ] = {ENTRY_TYPE_FUNC_POINTER,"","deconstruct",__deconstrcut,sizeof(void *)},
    [5 ] = {ENTRY_TYPE_VFUNC_POINTER,"","load_image",__load_image,sizeof(void *)},
    [6 ] = {ENTRY_TYPE_NORMAL_POINTER,"","String",NULL,sizeof(void *)},
    [7 ] = {ENTRY_TYPE_END},

};
REGISTER_CLASS("Image",image_class_info);

void test_obj_image()
{
    Image *image;
    allocator_t *allocator = allocator_get_default_alloc();

    image = OBJECT_NEW(allocator, Image,"");
    image->path->assign(image->path,"hello world");
}


