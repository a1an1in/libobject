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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/ui/Image.h>

static int __construct(Image *image, char *init_str)
{
    dbg_str(OBJ_DETAIL, "image construct, image addr:%p", image);
    image->path = (String *)OBJECT_NEW(((Obj *)image)->allocator, String, NULL);
    if (image->path == NULL) {
        dbg_str(DBG_ERROR, "construct image error");
        return -1;
    }

    return 0;
}

static int __deconstrcut(Image *image)
{
    dbg_str(OBJ_DETAIL, "image deconstruct, image addr:%p", image);
    object_destroy(image->path);

    return 0;
}

static int __load_image(Image *image, void *path)
{
    dbg_str(DBG_SUC, "image load image");
}

static void __set_size(Image *image, int width, int height)
{
    image->width = width;
    image->height = height;
}

static class_info_entry_t image_class_info[] = {
    Init_Obj___Entry(0 , Component, component),
    Init_Nfunc_Entry(1 , Image, construct, __construct),
    Init_Nfunc_Entry(2 , Image, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Image, set, NULL),
    Init_Vfunc_Entry(4 , Image, get, NULL),
    Init_Vfunc_Entry(5 , Image, load_image, __load_image),
    Init_Vfunc_Entry(6 , Image, draw, NULL),
    Init_Vfunc_Entry(7 , Image, set_name, NULL),
    Init_Vfunc_Entry(8 , Image, set_size, __set_size),
    Init_Vfunc_Entry(9 , Image, change_size, NULL),
    Init_End___Entry(10, Image),
};
REGISTER_CLASS(Image, image_class_info);
