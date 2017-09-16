/*
 * =====================================================================================
 *
 *       Filename:  libdbg_register_modules.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/13/2015 07:54:53 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdio.h>
#include <stdlib.h>
#include <libobject/utils/dbg/debug.h>
#include <libobject/attrib_priority.h>

extern void console_print_regester();
extern void log_print_regester();

__attribute__((constructor(ATTRIB_PRIORITY_LIBDBG_REGISTER_MODULES))) 
void libdbg_register_modules()
{
    ATTRIB_PRINT("constructor ATTRIB_PRIORITY_LIBDBG_REGISTER_MODULES=%d,register libdbg modules\n",
                 ATTRIB_PRIORITY_LIBDBG_REGISTER_MODULES);
    console_print_regester();
    //network_print_regester();
    log_print_regester();
}
