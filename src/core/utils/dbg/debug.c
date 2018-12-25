/*
 * =====================================================================================
 *
 *       Filename:  debug.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/24/2015 03:57:31 AM
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
#include <errno.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/dbg/debug_string.h>
#include <libobject/attrib_priority.h>
#include <libobject/core/utils/registry/registry.h>

debugger_t *debugger_gp;
debugger_module_t debugger_modules[MAX_DEBUGGER_MODULES_NUM];

void debugger_set_level_info(debugger_t *debugger, 
        uint32_t level, uint8_t color_value, const char *level_str)
{
    debugger->debug_level_info[level].color = color_value;
    memcpy(debugger->debug_level_info[level].level_str, level_str, strlen(level_str));
}
void debugger_set_level_infos(debugger_t *debugger)
{
    debugger_set_level_info(debugger, DBG_PANIC,     (DBG_PANIC_COLOR),    "DBG_PANIC");
    debugger_set_level_info(debugger, DBG_FATAL,     (DBG_FATAL_COLOR),    "DBG_FATAL");
    debugger_set_level_info(debugger, DBG_ERROR,     (DBG_ERROR_COLOR),    "DBG_ERROR");
    debugger_set_level_info(debugger, DBG_WARNNING,  (DBG_WARNNING_COLOR), "DBG_WARNNING");
    debugger_set_level_info(debugger, DBG_SUC,       (DBG_SUC_COLOR),      "DBG_SUC");
    debugger_set_level_info(debugger, DBG_CORRECT,   (DBG_CORRECT_COLOR),  "DBG_CORRECT");
    debugger_set_level_info(debugger, DBG_VIP,       (DBG_VIP_COLOR),      "DBG_VIP");
    debugger_set_level_info(debugger, DBG_FLOW,      (DBG_FLOW_COLOR),     "DBG_FLOW");
    debugger_set_level_info(debugger, DBG_IMPORTANT, (DBG_IMPORTANT_COLOR),"DBG_IMPORTANT");
    debugger_set_level_info(debugger, DBG_DETAIL,    (DBG_DETAIL_COLOR),   "DBG_DETAIL");
}
uint8_t debugger_get_level_color(debugger_t *debugger, uint32_t level)
{
    return debugger->debug_level_info[level].color;
}
uint8_t* debugger_get_level_str(debugger_t *debugger, uint32_t level)
{
    return debugger->debug_level_info[level].level_str;
}


void debugger_set_business(debugger_t *debugger, 
        uint32_t business_num, uint8_t on_off, uint8_t debug_level)
{
    debugger->debug_business[business_num].business_switch = on_off;
    debugger->debug_business[business_num].business_debug_level = debug_level;
}

void debugger_set_businesses(debugger_t *debugger)
{
    dictionary *d = debugger->d;;
    int bussiness_num, i;
#define MAX_STRING_LEN 50
    char switch_str[MAX_STRING_LEN];
    char level_str[MAX_STRING_LEN];
    int sw, lv;
    char buf[MAX_STRING_LEN];

    bussiness_num = iniparser_getint(d, (char *)"businesses:business_num", 0);
    sprintf(buf, "%d", MAX_DEBUG_BUSINESS_NUM);
    if(bussiness_num == 0){
        iniparser_setstr(d, (char *)"businesses", NULL); 
        iniparser_setstr(d, (char *)"businesses:business_num", buf); 
        for(i = 0; i < MAX_DEBUG_BUSINESS_NUM; i++){
            snprintf(switch_str, MAX_STRING_LEN,
                     "businesses:%s_switch",
                     debug_business_names[i]);

            snprintf(level_str, MAX_STRING_LEN,
                     "businesses:%s_level",
                     debug_business_names[i]);

            iniparser_setstr(d, switch_str, "1");
            if(i == 0){
                iniparser_setstr(d, level_str, "9");
                debugger_set_business(debugger, i, 1, 9);
            }else{
                iniparser_setstr(d, level_str, "6");
                debugger_set_business(debugger, i, 1, 6);
            }
        }
        FILE *f = fopen(debugger->ini_file_name, "w");
        iniparser_dump_ini(d, f);
        fclose(f);
    }else{
        for(i = 0; i < bussiness_num; i++){
            snprintf(switch_str, MAX_STRING_LEN,
                     "businesses:%s_switch",
                     debug_business_names[i]);

            snprintf(level_str, MAX_STRING_LEN,
                     "businesses:%s_level",
                     debug_business_names[i]);

            sw = iniparser_getint(d, switch_str, 1);
            lv = iniparser_getint(d, level_str, 6);
            debugger_set_business(debugger, i, sw, lv);
        }
    }
#undef MAX_BUSINESSES_SETTING_LEN
}

int debugger_is_business_switch_on(debugger_t *debugger, uint32_t business_num)
{
    return debugger->debug_business[business_num].business_switch == 1;
}

int debugger_get_business_level(debugger_t *debugger, uint32_t business_num)
{
    return debugger->debug_business[business_num].business_debug_level;
}

void debugger_init(debugger_t *debugger)
{
    if(debugger->dbg_ops->init)
        debugger->dbg_ops->init(debugger);
}

void debugger_destroy(debugger_t *debugger)
{
    if(debugger->dbg_ops->destroy)
        debugger->dbg_ops->destroy(debugger);
}

int debugger_dbg_str(debugger_t *debugger, uint32_t dbg_switch, const char *fmt, ...) 
{
#define MAX_FMT_STR_LEN 1024*4
    int ret = 0;
    va_list ap;
    uint32_t business_num = dbg_switch >> 8;
    uint8_t level = dbg_switch & 0xff;
    char fmt_str[MAX_FMT_STR_LEN];
    char *level_str;
    /* business print switch */
    if(!debugger_is_business_switch_on(debugger, business_num)){
        return -1;
    }
    /* business print level control */
    if(debugger_get_business_level(debugger, business_num) < level){
        return -1;
    }

    sync_lock(&debugger->lock, NULL);
    va_start(ap, fmt);
    vsnprintf(fmt_str, MAX_FMT_STR_LEN, fmt, ap);
    va_end(ap);
    level_str = (char *)debugger_get_level_str(debugger, level);
    ret = debugger->dbg_ops->dbg_string(debugger,
                                        level,
                                        "[%s]--%s",
                                        level_str,
                                        fmt_str);
    sync_unlock(&debugger->lock);

    return ret;
#undef MAX_FMT_STR_LEN 
}

int debugger_dbg_buf(debugger_t *debugger, 
                     uint32_t dbg_switch, 
                     const char* const_str, 
                     uint8_t *buf, 
                     uint32_t buf_len, 
                     const char *fmt, ...) 
{
#define MAX_BUFFER_STR_LEN 1024*4
#define MAX_FMT_STR_LEN 1024
    int ret = 0;
    va_list ap;
    uint32_t business_num = dbg_switch >> 8;
    uint8_t level = dbg_switch & 0xff;
    char buffer_str[MAX_BUFFER_STR_LEN];
    char fmt_str[MAX_FMT_STR_LEN];
    char *level_str;

    if(!debugger_is_business_switch_on(debugger, business_num)){
        return -1;
    }
    if(debugger_get_business_level(debugger, business_num) < level){
        return -1;
    }
    debug_string_buf_to_str(buf, buf_len, buffer_str, MAX_BUFFER_STR_LEN);
    va_start(ap, fmt);
    vsnprintf(fmt_str, MAX_FMT_STR_LEN, fmt, ap);
    va_end(ap);
    level_str = (char*)debugger_get_level_str(debugger, level);

    ret = debugger->dbg_ops->dbg_string(debugger, 
                                        level, 
                                        "[%s]--[%s%s]--%s", 
                                        level_str, 
                                        const_str, 
                                        buffer_str, 
                                        fmt_str);

    return ret;
#undef MAX_BUFFER_STR_LEN
#undef MAX_FMT_STR_LEN 
}

debugger_t *debugger_creator(char *ini_file_name, uint8_t lock_type)
{
    debugger_t *debugger;
    int8_t type;
    dictionary *d;
    FILE *f;
    int err;

    debugger =(debugger_t *)malloc(sizeof(debugger_t));

    debugger->d = d = iniparser_new(ini_file_name);
    memcpy(debugger->ini_file_name, ini_file_name, strlen(ini_file_name));

    type = iniparser_getint(d, (char *)"debugger:type", 0);
    if(access(debugger->ini_file_name, F_OK)){
        printf("ini file %s not exist\n", debugger->ini_file_name);
        f = fopen(debugger->ini_file_name, "w");
        if(f == NULL){
            err = errno;
            console_str("open ini file failed, errno=%d", err);
            exit(1);
        }
        iniparser_setstr(d, (char *)"debugger", NULL); 
        iniparser_setstr(d, (char *)"debugger:type", "0");
        iniparser_dump_ini(d, f);
        fclose(f);
    }
    debugger->debugger_type = type;
    debugger->dbg_ops = &debugger_modules[type].dbg_ops;
    debugger->lock_type = lock_type;
    sync_lock_init(&debugger->lock, debugger->lock_type);

    debugger_set_level_infos(debugger);
    debugger_set_businesses(debugger);

    return debugger;

}

int debugger_constructor()
{
    char *file_name;
    ATTRIB_PRINT("REGISTRY_CTOR_PRIORITY=%d, construct debugger\n", 
                 REGISTRY_CTOR_PRIORITY_DEBUGGER);

#if (defined(UNIX_USER_MODE) || defined(LINUX_USER_MODE) || defined(IOS_USER_MODE) || defined(MAC_USER_MODE))
    file_name = "/tmp/dbg.ini";

    debugger_gp = debugger_creator(file_name, PTHREAD_MUTEX_LOCK);
    debugger_init(debugger_gp);

#elif defined(ANDROID_USER_MODE)
    /*
     *file_name = "/storage/sdcard/dbg.ini";
     */
#else
    file_name= "dbg.ini";

    debugger_gp = debugger_creator(file_name, PTHREAD_MUTEX_LOCK);
    debugger_init(debugger_gp);
#endif

    return 0;
}
REGISTER_CTOR_FUNC(REGISTRY_CTOR_PRIORITY_DEBUGGER, debugger_constructor);

int debugger_destructor()
{
    ATTRIB_PRINT("REGISTRY_DTOR_PRIORITY=%d, debugger destructor\n", 
                 REGISTRY_DTOR_PRIORITY_DEBUGGER);
#if (!defined(ANDROID_USER_MODE))
    debugger_destroy(debugger_gp);
#endif

    return 0;
}
REGISTER_DTOR_FUNC(REGISTRY_DTOR_PRIORITY_DEBUGGER, debugger_destructor);

