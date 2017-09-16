/*
 * =====================================================================================
 *
 *       Filename:  debug_string.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/25/2015 11:42:52 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __DEBUG_STRING_H__
#define __DEBUG_STRING_H__
#include "basic_types.h"
uint32_t debug_string_buf_to_str(uint8_t *buf_addr,size_t buf_len,char *str,size_t str_len);
int debug_string_itoa(int val, char* buf,int radix);
#endif
