/**
 * @file Lock_Mutex.c
 * @synopsis 
 * @author a1an1in@sina.com
 * @version 
 * @date 2017-10-07
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
#include <unistd.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/timeval/timeval.h>
#include "Mutex_Lock.h"

static int __construct(Mutex_Lock *lock, char *init_str)
{
    dbg_str(OBJ_DETAIL, "muxtex lock construct, lock addr:%p", lock);

    return pthread_mutex_init(&lock->mutex, NULL);
}

static int __deconstrcut(Mutex_Lock *lock)
{
    dbg_str(OBJ_DETAIL, "mutex lock deconstruct, lock addr:%p", lock);

    return pthread_mutex_destroy(&lock->mutex);
}

static int __lock(Mutex_Lock *lock)
{
    dbg_str(OBJ_DETAIL, "mutex lock");
    return pthread_mutex_lock(&lock->mutex);
}

static int __trylock(Mutex_Lock *lock)
{
    dbg_str(OBJ_DETAIL, "mutex trylock");
    return pthread_mutex_trylock(&lock->mutex);
}

static int __unlock(Mutex_Lock *lock)
{
    dbg_str(OBJ_DETAIL, "mutex unlock");
    return pthread_mutex_unlock(&lock->mutex);
}

static class_info_entry_t lock_class_info[] = {
    Init_Obj___Entry(0 , Lock, parent),
    Init_Nfunc_Entry(1 , Mutex_Lock, construct, __construct),
    Init_Nfunc_Entry(2 , Mutex_Lock, deconstruct, __deconstrcut),
    Init_Vfunc_Entry(3 , Mutex_Lock, lock, __lock),
    Init_Vfunc_Entry(4 , Mutex_Lock, trylock, __trylock),
    Init_Vfunc_Entry(5 , Mutex_Lock, unlock, __unlock),
    Init_End___Entry(6 , Mutex_Lock),
};
REGISTER_CLASS(Mutex_Lock, lock_class_info);
