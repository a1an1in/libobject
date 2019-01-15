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
#include <libobject/core/utils/config/config.h>
#include <libobject/core/utils/timeval/timeval.h>
#include <libobject/event/event_base.h>
#include <libobject/core/mutex_lock.h>

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

static int __set(Mutex_Lock *lock, char *attrib, void *value)
{
    if (strcmp(attrib, "set") == 0) {
        lock->set = value;
    } else if (strcmp(attrib, "get") == 0) {
        lock->get = value;
    } else if (strcmp(attrib, "construct") == 0) {
        lock->construct = value;
    } else if (strcmp(attrib, "deconstruct") == 0) {
        lock->deconstruct = value;
    }
    else if (strcmp(attrib, "lock") == 0) {
        lock->lock = value;
    } else if (strcmp(attrib, "trylock") == 0) {
        lock->trylock = value;
    } else if (strcmp(attrib, "unlock") == 0) {
        lock->unlock = value;
    }
    else {
        dbg_str(OBJ_DETAIL, "lock set, not support %s setting", attrib);
    }

    return 0;
}

static void *__get(Mutex_Lock *obj, char *attrib)
{
    if (strcmp(attrib, "") == 0) {
    } else {
        dbg_str(OBJ_WARNNING, "lock get, \"%s\" getting attrib is not supported", attrib);
        return NULL;
    }
    return NULL;
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
    [0] = {ENTRY_TYPE_OBJ, "Lock", "parent", NULL, sizeof(void *)}, 
    [1] = {ENTRY_TYPE_FUNC_POINTER, "", "set", __set, sizeof(void *)}, 
    [2] = {ENTRY_TYPE_FUNC_POINTER, "", "get", __get, sizeof(void *)}, 
    [3] = {ENTRY_TYPE_FUNC_POINTER, "", "construct", __construct, sizeof(void *)}, 
    [4] = {ENTRY_TYPE_FUNC_POINTER, "", "deconstruct", __deconstrcut, sizeof(void *)}, 
    [5] = {ENTRY_TYPE_VFUNC_POINTER, "", "lock", __lock, sizeof(void *)}, 
    [6] = {ENTRY_TYPE_VFUNC_POINTER, "", "trylock", __trylock, sizeof(void *)}, 
    [7] = {ENTRY_TYPE_VFUNC_POINTER, "", "unlock", __unlock, sizeof(void *)}, 
    [8] = {ENTRY_TYPE_END}, 
};
REGISTER_CLASS("Mutex_Lock", lock_class_info);

int test_mutex_lock()
{
    Lock *lock;
    allocator_t *allocator = allocator_get_default_alloc();
    char *set_str;
    cjson_t *root, *e, *s;
    char buf[2048];

    lock = OBJECT_NEW(allocator, Mutex_Lock, NULL);

    lock->lock(lock);
    lock->unlock(lock);

    object_destroy(lock);

    return 1;
}
REGISTER_STANDALONE_TEST_FUNC(test_mutex_lock);
