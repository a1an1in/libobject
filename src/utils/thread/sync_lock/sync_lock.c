/*
 * =====================================================================================
 *
 *       Filename:  sync_lock.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/24/2015 02:08:23 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
/*  
 * Copyright (c) 2015-2020 alan lin <a1an1in@sina.com>
 *  
 *  
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
#include <stdlib.h>
#include <libobject/cutils_re.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/utils/thread/sync_lock.h>
#include <libobject/utils/thread/posix_thread_mutex.h>
#include <libobject/utils/thread/posix_thread_rwlock.h>
#include <libobject/utils/thread/windows_mutex.h>
#include <libobject/attrib_priority.h>


sync_lock_module_t sync_lock_modules[SYNC_LOCK_TYPE_MAX_NUM];

__attribute__((constructor(ATTRIB_PRIORITY_SYNC_LOCK_REGISTER_MODULES))) void
sync_lock_register_modules()
{
    /*
     *sync_lock_module_t sync_lock_modules[SYNC_LOCK_TYPE_MAX_NUM];
     */
    /*
     *memset(&sync_lock_modules[PTHREAD_RWLOCK],0,sizeof(sync_lock_module_t));
     */
    ATTRIB_PRINT("constructor ATTRIB_PRIORITY_SYNC_LOCK_REGISTER_MODULES=%d,register sync lock modules\n",
                 ATTRIB_PRIORITY_SYNC_LOCK_REGISTER_MODULES);
#ifdef UNIX_LIKE_USER_MODE
    linux_user_mode_pthread_mutex_register();
    linux_user_mode_pthread_rwlock_register();
#endif
#ifdef WINDOWS_USER_MODE
    windows_user_mode_mutex_register();
#endif
}


