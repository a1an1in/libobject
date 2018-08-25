/*
 * =====================================================================================
 *
 *       Filename:  log.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/24/2015 03:48:51 AM
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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/dbg/debug_log.h>
#include <libobject/core/utils/registry/registry.h>

/*init socket*/
void log_print_init(debugger_t *debugger)
{
    debug_log_prive_t *log_priv = &debugger->priv.log;
    FILE *fp;
    dictionary *d = debugger->d;;
    char *file_name;
    char default_log_name[] = "dbg_log.txt";

    printf("debug log init\n");
    file_name = iniparser_getstr(d, (char *)"log:log_file_name");
    if(file_name){
        memcpy(log_priv->log_file_name, file_name, strlen(file_name));
    }else{
        memcpy(log_priv->log_file_name, default_log_name, strlen(default_log_name));
        iniparser_setstr(d, (char *)"Log", NULL); 
        iniparser_setstr(d, (char *)"Log:log_file_name", log_priv->log_file_name);

        FILE *f = fopen(debugger->ini_file_name, "w");
        iniparser_dump_ini(d, f);
        fclose(f);
    }

    fp = fopen(log_priv->log_file_name, "ab+");
    if(fp == NULL)
    {
        perror("log file fopen()");
        exit(1);
    }
    log_priv->fp = fp;

    /*
     *pthread_mutex_init(&log_priv->log_file_lock, NULL);
     */
    printf("run at here.\n");
    sync_lock_init(&log_priv->log_file_lock, debugger->lock_type);
    printf("debug log init end\n");
}
uint32_t log_print_write_log(FILE *fp, char *str)
{
    uint32_t ret;

    ret = fprintf(fp, "%s\n", str);
    fflush(fp);

    return ret;
}

void log_print_destroy(debugger_t *debugger)
{
    debug_log_prive_t *log_priv = &debugger->priv.log;
    /*
     *pthread_mutex_destroy(&log_priv->log_file_lock);
     */
    sync_lock_destroy(&log_priv->log_file_lock);
    fclose(log_priv->fp);
}

int log_print_print_str_vl(debugger_t *debugger, 
                           size_t level, const char *fmt, va_list vl)
{
#define MAX_LOG_PRINT_BUFFER_LEN 1024*4
    char buffer_str[MAX_LOG_PRINT_BUFFER_LEN];
    size_t ret = 0, offset = 0;
    debug_log_prive_t *log_priv = &debugger->priv.log;

    level = 0;
    /*
     *pthread_mutex_t *lock = &log_priv->log_file_lock;
     *pthread_mutex_lock(lock);
     */
    sync_lock(&log_priv->log_file_lock, NULL);
    memset(buffer_str, '\0', MAX_LOG_PRINT_BUFFER_LEN);
    offset = vsnprintf(buffer_str, MAX_LOG_PRINT_BUFFER_LEN, fmt, vl);
    ret = log_print_write_log(log_priv->fp, buffer_str);
    sync_unlock(&log_priv->log_file_lock);
    /*
     *pthread_mutex_unlock(&log_priv->log_file_lock);
     */

    return ret;
#undef MAX_LOG_PRINT_BUFFER_LEN 
}

int log_print_print_str(debugger_t *debugger, size_t level, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = log_print_print_str_vl(debugger, level, fmt, ap);
    va_end(ap);

    return ret;
}

int log_print_regester()
{
    debugger_module_t dm={
        .dbg_ops ={
            .dbg_string_vl = log_print_print_str_vl, 
            .dbg_string    = log_print_print_str, 
            .init          = log_print_init, 
            .destroy       = log_print_destroy, 
        }
    };
    ATTRIB_PRINT("REGISTRY_CTOR_PRIORITY=%d, register dbg log print module\n", 
                 REGISTRY_CTOR_PRIORITY_LIBDBG_REGISTER_MODULES);
    memcpy(&debugger_modules[DEBUGGER_TYPE_LOG], &dm, sizeof(debugger_module_t));

    return 0;
}
REGISTER_CTOR_FUNC(REGISTRY_CTOR_PRIORITY_LIBDBG_REGISTER_MODULES, 
                   log_print_regester);
