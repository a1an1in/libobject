/**
 * @Windows_Pipe Windows_Pipe.c
 * @Synopsis  
 * @author alan lin
 * @version 
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
#if (defined(WINDOWS_USER_MODE))

#include <stdio.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/try.h>
#include "Windows_Pipe.h"

#define DEFAULT_Windows_Pipe_SIZE 1024

static int __construct(Windows_Pipe *pipe, char *init_str)
{
    allocator_t *allocator = ((Obj *)pipe)->allocator;

    return 0;
}

static int __deconstrcut(Windows_Pipe *pipe)
{
    allocator_t *allocator = ((Obj *)pipe)->allocator;

    CloseHandle(pipe->read_handle);
    CloseHandle(pipe->write_handle);

    return 0;
}

static int __init(Windows_Pipe *pipe)
{
    SECURITY_ATTRIBUTES sa;
    HANDLE hPipeInputRead = NULL;
    HANDLE hPipeInputWrite = NULL;
    int ret;

    TRY {
        sa.nLength = sizeof(sa);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = NULL;

        CreatePipe(&pipe->read_handle, &pipe->write_handle, &sa, 0);
    } CATCH (ret) {}

    return ret;
}

static int __read(Windows_Pipe *pipe, void *dst, int len)
{
    DWORD read_len = 0;
    int ret;

    TRY {
        EXEC(ReadFile(pipe->read_handle, dst, len, &read_len, NULL));
        THROW(read_len);
    } CATCH (ret) {}

    return ret;
}

static int __write(Windows_Pipe *pipe, void *src, int len)
{
    DWORD write_len = 0;
    int ret;

    TRY {
        EXEC(WriteFile(pipe->write_handle, src, len, &write_len, NULL));
        THROW(write_len);
    } CATCH (ret) {}

    return ret;
}

static class_info_entry_t Windows_Pipe_class_info[] = {
    Init_Obj___Entry(0 , Pipe, parent),
    Init_Nfunc_Entry(1 , Windows_Pipe, construct, __construct),
    Init_Nfunc_Entry(2 , Windows_Pipe, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Windows_Pipe, set, NULL),
    Init_Vfunc_Entry(4 , Windows_Pipe, get, NULL),
    Init_Vfunc_Entry(5 , Windows_Pipe, init, __init),
    Init_Vfunc_Entry(6 , Windows_Pipe, read, __read),
    Init_Vfunc_Entry(7 , Windows_Pipe, write, __write),
    Init_End___Entry(8 , Windows_Pipe),
};
REGISTER_CLASS("Windows_Pipe", Windows_Pipe_class_info);
#endif