/*
 * =====================================================================================
 *
 *       Filename:  console.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/24/2015 03:49:44 AM
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
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/user_mode.h>

int console_print_print_str_vl(debugger_t *debugger, 
        size_t level, const char *fmt, va_list vl)
{
#define MAX_STR_LEN 1024
    char dest[MAX_STR_LEN];
    size_t offset=0 , ret = 0;

    memset(dest, '\0', MAX_STR_LEN);
    offset = vsnprintf(dest, MAX_STR_LEN, fmt, vl);

#if (defined(UNIX_USER_MODE) || defined(LINUX_USER_MODE) || defined(ANDROID_USER_MODE) || defined(IOS_USER_MODE) || defined(MAC_USER_MODE))
    int reverse_color_flag, background_color, front_color, high_light_flag;
    char high_light_value[MAX_STR_LEN];
    char reverse_color_value[MAX_STR_LEN];
    size_t color_value;

    color_value        = debugger_get_level_color(debugger, level);
    reverse_color_flag = color_value&0x1;
    background_color   = ((color_value&0x70)>>4);
    front_color        = ((color_value&0xe)>>1);
    high_light_flag    = ((color_value>>7)&1);

    if(high_light_flag == 1){
        strcpy(high_light_value, "\33[1m");
    }else{
        strcpy(high_light_value, "");
    }
    if(reverse_color_flag == 1){
        strcpy(reverse_color_value, "\33[7m");
    }else{
        strcpy(reverse_color_value, "");
    }
    if((color_value & 0xfe) == 0xe & high_light_flag == 0){
        printf("\33[0m\33[%dm\33[%dm", 49, 30);
        printf("%s", dest);
        printf("\33[0m\033[K");
    }else{
        printf("\33[0m%s%s\33[%dm\33[%dm",
                high_light_value, reverse_color_value,
                background_color+40, front_color+30);
        printf("%s", dest);
        printf("\33[0m\033[K");
    }
    printf("\n");

    return strlen(dest);
#elif defined(WINDOWS_USER_MODE)
    printf("%s\n", dest);
#endif

    return ret;
#undef MAX_STR_LEN 
}
int console_print_print(debugger_t *debugger, size_t level, const char *fmt, ...)
{
    int ret;
    va_list ap;

    va_start(ap, fmt);
    ret = console_print_print_str_vl(debugger, level, fmt, ap);
    va_end(ap);

    return ret;
}

int console_print_regester()
{
    debugger_module_t dm={
        .dbg_ops ={
            .dbg_string_vl = console_print_print_str_vl, 
            .dbg_string    = console_print_print, 
            .init          = NULL, 
            .destroy       = NULL, 
        }
    };
    ATTRIB_PRINT("REGISTRY_CTOR_PRIORITY=%d, register dbg console print module\n", 
                 REGISTRY_CTOR_PRIORITY_LIBDBG_REGISTER_MODULES);

    memcpy(&debugger_modules[DEBUGGER_TYPE_CONSOLE], &dm, sizeof(debugger_module_t));

    return 0;
}
REGISTER_CTOR_FUNC(REGISTRY_CTOR_PRIORITY_LIBDBG_REGISTER_MODULES,
                   console_print_regester);
