/*
 * =====================================================================================
 *
 *       Filename:  debug_log.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/25/2015 02:42:15 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __DEBUG_LOG_H__
#define __DEBUG_LOG_H__
#include <stdio.h>
#include <libobject/core/utils/thread/sync_lock.h>
/*
 *#include <pthread.h>
 */

typedef struct debug_log_prive{
#define MAX_DEBUG_LOG_FILE_NAME_LEN 50
	FILE *fp;
	char log_file_name[MAX_DEBUG_LOG_FILE_NAME_LEN];
	/*
	 *pthread_mutex_t log_file_lock;
	 */
	sync_lock_t log_file_lock;
#undef MAX_DEBUG_LOG_FILE_NAME_LEN 
}debug_log_prive_t;
int log_print_regester();

#endif
