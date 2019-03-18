/**
 * @file subject.c
 * @synopsis 
 * @author alan(a1an1in@sina.com)
 * @version 1
 * @date 2016-11-21
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
#include <libobject/ui/subject.h>

static int __construct(Subject *subject, char *init_str)
{
    dbg_str(OBJ_DETAIL, "subject construct, subject addr:%p", subject);

    subject->x_bak      = subject->x;
    subject->y_bak      = subject->y;
    subject->width_bak  = subject->width;
    subject->height_bak = subject->height;

    return 0;
}

static int __deconstrcut(Subject *subject)
{
    dbg_str(OBJ_DETAIL, "subject deconstruct, subject addr:%p", subject);

    return 0;
}

static int __move(Subject *subject)
{
    dbg_str(OBJ_DETAIL, "subject move");
}

static int __show(Subject *subject)
{
    dbg_str(OBJ_DETAIL, "subject show");
}

static int __add_x_speed(Subject *subject, float v) 
{
    dbg_str(OBJ_DETAIL, "__add_x_speed");
}

static int __add_y_speed(Subject *subject, float v) 
{
    dbg_str(OBJ_DETAIL, "__add_x_speed");
}

static int __is_touching(Subject *me, Subject *subject)
{
    dbg_str(OBJ_DETAIL, "__is_touching");
}

static int __set(Subject *subject, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        subject->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        subject->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        subject->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        subject->deconstruct = value;
    } else if (strcmp(attrib, "move") == 0) {
        subject->move = value;
    } else if (strcmp(attrib, "show") == 0) {
        subject->show = value;
    } else if (strcmp(attrib, "add_x_speed") == 0) {
        subject->add_x_speed = value;
    } else if (strcmp(attrib, "add_y_speed") == 0) {
        subject->add_y_speed = value;
    } else if (strcmp(attrib, "is_touching") == 0) {
        subject->is_touching = value;
    } else if (strcmp(attrib, "x") == 0) {
        subject->x = *((uint32_t *)value);
    } else if (strcmp(attrib, "y") == 0) {
        subject->y = *((uint32_t *)value);
    } else if (strcmp(attrib, "width") == 0) {
        subject->width = *((uint32_t *)value);
    } else if (strcmp(attrib, "height") == 0) {
        subject->height = *((uint32_t *)value);
    } else if (strcmp(attrib, "x_speed") == 0) {
        subject->x_speed = *((int *)value);
        dbg_str(DBG_DETAIL, "set x_speed =%f", subject->x_speed);
    } else if (strcmp(attrib, "y_speed") == 0) {
        subject->y_speed = *((int *)value);
        dbg_str(DBG_DETAIL, "set y_speed =%f", subject->y_speed);
    } else {
        dbg_str(OBJ_WARNNING, "subject set,  \"%s\" setting is not support", attrib);
    }

    return 0;
}

static void * __get(Subject *subject, char *attrib)
{
    if (strcmp(attrib, "x") == 0) 
        return &subject->x;
    else if (strcmp(attrib, "y") == 0) 
        return &subject->y;
    else if (strcmp(attrib, "width") == 0) 
        return &subject->width;
    else if (strcmp(attrib, "height") == 0) 
        return &subject->height;
    else if (strcmp(attrib, "x_speed") == 0) 
        return &subject->x_speed;
    else if (strcmp(attrib, "y_speed") == 0) {
        return &subject->y_speed;
    } else {
        dbg_str(OBJ_WARNNING, "subject get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t subject_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Nfunc_Entry(1 , Subject, construct, __construct),
    Init_Nfunc_Entry(2 , Subject, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Subject, set, NULL),
    Init_Vfunc_Entry(4 , Subject, get, NULL),
    Init_Vfunc_Entry(5 , Subject, move, __move),
    Init_Vfunc_Entry(6 , Subject, show, __show),
    Init_Vfunc_Entry(7 , Subject, add_x_speed, __add_x_speed),
    Init_Vfunc_Entry(8 , Subject, add_y_speed, __add_y_speed),
    Init_Vfunc_Entry(9 , Subject, is_touching, __is_touching),
    Init_U32___Entry(10, Subject, x, NULL),
    Init_U32___Entry(11, Subject, y, NULL),
    Init_U32___Entry(12, Subject, width, NULL),
    Init_U32___Entry(13, Subject, height, NULL),
    Init_Float_Entry(14, Subject, x_speed, NULL),
    Init_Float_Entry(15, Subject, y_speed, NULL),
    Init_End___Entry(16),
};
REGISTER_CLASS("Subject", subject_class_info);
