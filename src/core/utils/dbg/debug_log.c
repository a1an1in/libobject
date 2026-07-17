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
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/dbg/debug_log.h>
#include <libobject/core/utils/registry/registry.h>

#define DEFAULT_LOG_FILE_NAME       "/home/alan/.xtools/logs"
#define LOG_FILE_NAME_BUF_SIZE      256

/*
 * Build the daily log file path.
 * DBG_LOG_DIR is the logs directory (e.g. "/home/alan/.xtools/httpd/logs"),
 * files are named with date suffix.
 * e.g. "/home/alan/.xtools/httpd/logs/log-2026-07-17"
 */
static int log_build_date_path(debug_log_prive_t *log_priv,
                               char *buf, size_t buf_size,
                               int year, int mon, int mday)
{
    return snprintf(buf, buf_size, "%s/log-%04d-%02d-%02d",
                    log_priv->log_file_base,
                    year, mon, mday);
}

/*
 * Check if the date has changed. If so, close the old file and open a new one.
 * Must be called with log_file_lock held.
 */
static void log_check_date_rotation(debug_log_prive_t *log_priv)
{
    struct tm *local_time;
    struct timeval tv;
    char new_path[LOG_FILE_NAME_BUF_SIZE];
    FILE *new_fp;

    if (!log_priv->rotate_on_date) {
        return; /* daily rotation disabled, use single file */
    }

    /* get current date */
    gettimeofday(&tv, NULL);
    local_time = localtime(&tv.tv_sec);

    int year  = local_time->tm_year + 1900;
    int mon   = local_time->tm_mon + 1;
    int mday  = local_time->tm_mday;

    /* check if date changed */
    if (year == log_priv->current_year &&
        mon  == log_priv->current_mon &&
        mday == log_priv->current_mday) {
        return; /* same date, no rotation needed */
    }

    /* build new file path */
    log_build_date_path(log_priv, new_path, sizeof(new_path),
                        year, mon, mday);

    /* open new log file */
    new_fp = fopen(new_path, "ab+");
    if (new_fp == NULL) {
        perror("log_date_rotate: fopen");
        return; /* keep writing to old file on failure */
    }

    /* swap to new file */
    if (log_priv->fp) {
        fclose(log_priv->fp);
    }
    log_priv->fp = new_fp;
    snprintf(log_priv->log_file_name, sizeof(log_priv->log_file_name),
             "%s", new_path);

    /* update current date */
    log_priv->current_year  = year;
    log_priv->current_mon   = mon;
    log_priv->current_mday  = mday;

    printf("log rotated to: %s\n", log_priv->log_file_name);
}

/*init log file*/
void log_print_init(debugger_t *debugger)
{
    debug_log_prive_t *log_priv = &debugger->priv.log;
    FILE *fp;
    dictionary *d = debugger->d;;
    char *file_name;
    char *env_name;
    int rotate_on_date;
    struct tm *local_time;
    struct timeval tv;
    char log_path[LOG_FILE_NAME_BUF_SIZE];
    char log_dir[LOG_FILE_NAME_BUF_SIZE];
    char *p;

    printf("debug log init\n");

    /* --- read log file name: env var > ini config > default --- */
    env_name = getenv("DBG_LOG_DIR");
    if (env_name) {
        snprintf(log_priv->log_file_base, sizeof(log_priv->log_file_base),
                 "%s", env_name);
        printf("debug log: using env DBG_LOG_DIR=%s\n", env_name);
    } else {
        file_name = iniparser_getstr(d, (char *)"log:log_dir");
        if (file_name) {
            snprintf(log_priv->log_file_base, sizeof(log_priv->log_file_base),
                     "%s", file_name);
        } else {
            snprintf(log_priv->log_file_base, sizeof(log_priv->log_file_base),
                     "%s", DEFAULT_LOG_FILE_NAME);
            iniparser_setstr(d, (char *)"Log", NULL);
            iniparser_setstr(d, (char *)"Log:log_dir",
                             log_priv->log_file_base);

            FILE *f = fopen(debugger->ini_file_name, "w");
            iniparser_dump_ini(d, f);
            fclose(f);
        }
    }

    /* --- ensure log directory exists (create parent dirs if needed) --- */
    snprintf(log_dir, sizeof(log_dir), "%s", log_priv->log_file_base);
    /* create directory hierarchy: /home/alan/.xtools/httpd/logs */
    for (p = log_dir + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';
            mkdir(log_dir, 0755);
            *p = '/';
        }
    }
    mkdir(log_dir, 0755);

    /* --- read daily rotation config --- */
    rotate_on_date = iniparser_getint(d, (char *)"log:rotate_on_date", 1);
    log_priv->rotate_on_date = (rotate_on_date != 0) ? 1 : 0;

    printf("debug log: base=%s, rotate_on_date=%d\n",
           log_priv->log_file_base, log_priv->rotate_on_date);

    /* --- determine the actual log file path --- */
    gettimeofday(&tv, NULL);
    local_time = localtime(&tv.tv_sec);
    log_priv->current_year  = local_time->tm_year + 1900;
    log_priv->current_mon   = local_time->tm_mon + 1;
    log_priv->current_mday  = local_time->tm_mday;

    if (log_priv->rotate_on_date) {
        log_build_date_path(log_priv, log_path, sizeof(log_path),
                            log_priv->current_year,
                            log_priv->current_mon,
                            log_priv->current_mday);
    } else {
        snprintf(log_path, sizeof(log_path), "%s",
                 log_priv->log_file_base);
    }

    snprintf(log_priv->log_file_name, sizeof(log_priv->log_file_name),
             "%s", log_path);

    /* --- open log file --- */
    fp = fopen(log_priv->log_file_name, "ab+");
    if (fp == NULL) {
        perror("log file fopen()");
        exit(1);
    }
    log_priv->fp = fp;

    printf("run at here.\n");
    sync_lock_init(&log_priv->log_file_lock, debugger->lock_type);
    printf("debug log init end, log file: %s\n", log_priv->log_file_name);
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

    /* check daily rotation before writing */
    log_check_date_rotation(log_priv);

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
