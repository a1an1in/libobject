/**
 * @File Stream.c
 * @Synopsis  
 * @author alan lin
 * @version 1
 * @date 2019-01-13
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
#include <libobject/io/stream.h>

static int __set(Stream *stream, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        stream->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        stream->get = value;
    }
    else if (strcmp(attrib, "construct") == 0) {
        stream->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        stream->deconstruct = value;
    } else if (strcmp(attrib, "read") == 0) {
        stream->read = value;
    } else if (strcmp(attrib, "write") == 0) {
        stream->write = value;
    } else {
        dbg_str(EV_DETAIL,"stream set, not support %s setting",attrib);
    }

    return 0;
}

static void *__get(Stream *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(EV_WARNNING,"stream get, \"%s\" getting attrib is not supported",attrib);
        return NULL;
    }
    return NULL;
}

static class_info_entry_t stream_class_info[] = {
    Init_Obj___Entry(0 , Obj, obj),
    Init_Vfunc_Entry(1 , Stream, set, NULL),
    Init_Vfunc_Entry(2 , Stream, get, NULL),
    Init_Vfunc_Entry(3 , Stream, read, NULL),
    Init_Vfunc_Entry(4 , Stream, write, NULL),
    Init_End___Entry(5 ),
};
REGISTER_CLASS("Stream",stream_class_info);
