#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "basic_types.h"
#include "debug_console.h"
#include "debug_network.h"
#include "debug_log.h"
#include <libobject/user_mode.h>
#include <libobject/core/utils/ini/iniparser.h>


enum debugger_type{
	DEBUGGER_TYPE_CONSOLE = 0,
	DEBUGGER_TYPE_LOG,
	DEBUGGER_TYPE_NETWORK,
};

enum debug_level_enum{
	DBG_PANIC=0, 		
	DBG_FATAL, 		
	DBG_ERROR ,		
	DBG_WARNNING,
	DBG_SUC ,		
	DBG_CORRECT,
	DBG_VIP, 	
	DBG_FLOW,
	DBG_IMPORTANT, 		
	DBG_DETAIL,
	DBG_MAX_LEVEL
};


#include "debug_businesses.h"

typedef struct debug_level_info debug_level_info_t;
typedef struct debugger debugger_t;
typedef struct debugger_module debugger_module_t;


struct debugger_opperations{
	int (*dbg_string_vl)(debugger_t *debugger,size_t level,const char *fmt,va_list vl);
	int (*dbg_string)(debugger_t *debugger,size_t level,const char *fmt,...);
	void (*init)(debugger_t *debugger);
	void (*destroy)(debugger_t *debugger);
};
struct debugger_module{
	struct debugger_opperations dbg_ops;
};


struct debug_level_info{
#define MAX_PRINT_LEVEL_NAME_LEN 20
	uint8_t color;
	uint8_t level_str[MAX_PRINT_LEVEL_NAME_LEN];
#undef MAX_PRINT_LEVEL_NAME_LEN 
};
struct debug_business{
	uint8_t business_switch;
	uint8_t business_debug_level;
};

struct debugger{
#define MAX_INI_FILE_NAME_LEN 50
	debug_level_info_t debug_level_info[DBG_MAX_LEVEL];
	uint8_t main_switch;
	uint8_t level;
	uint8_t debugger_type;
	char ini_file_name[MAX_INI_FILE_NAME_LEN];
	dictionary *d;
	struct debug_business debug_business[MAX_DEBUG_BUSINESS_NUM];
	struct debugger_opperations *dbg_ops;
	uint8_t lock_type;
	union debugger_priv{
        debug_network_prive_t net;
		debug_log_prive_t log;
	}priv;
	sync_lock_t lock;
#undef MAX_INI_FILE_NAME_LEN
};


extern debugger_t *debugger_gp;
#define MAX_DEBUGGER_MODULES_NUM 3
extern debugger_module_t debugger_modules[MAX_DEBUGGER_MODULES_NUM];

uint8_t debugger_get_level_color(debugger_t *debugger,uint32_t level);
int debugger_get_business_level(debugger_t *debugger,uint32_t business_num);

void debugger_set_business(debugger_t *debugger,
                           uint32_t business_num,
                           uint8_t on_off,uint8_t debug_level);

int debugger_dbg_str(debugger_t *debugger,
                     uint32_t dbg_switch,const char *fmt,...);

int debugger_dbg_buf(debugger_t *debugger,
                     uint32_t dbg_switch,
                     const char* const_str, 
                     uint8_t *buf,
                     uint32_t buf_len, const char *fmt,...);

debugger_t *debugger_creator(char *ini_file_name,uint8_t lock_type);
void debugger_init(debugger_t *debugger);
void debugger_destroy(debugger_t *debugger);

static inline char *
extract_filename_in_macro(char *macro)
{
    char * index;

    index = strrchr(macro, '/');
    if (index != NULL) {
        return index + 1;
    } else if ((index = strrchr(macro, '\\')) != NULL) {
        return index + 1;
    } else {
        return macro;
    }

    return NULL;
}

/*color*/
#define BLACK 				0x0
#define RED 				0x1
#define GREEN 				0x2
#define YELLOW 				0x3
#define BLUE 				0x4
#define PURPLE 				0x5
#define DARK_GREEN 			0x6
#define WHITE				0x7

/*reverse backgrand and font flag*/
#define REVERSE_COLOR_FLAG  1
#define HIGHLIGHT_FLAG 	1

/*backgrand and font color*/
#define BGC(color) color << 4
#define FTC(color) color << 1

/*debug level color*/
#define DBG_PANIC_COLOR 		HIGHLIGHT_FLAG << 7 | BGC(RED)    | FTC(WHITE)  
#define DBG_ERROR_COLOR 		HIGHLIGHT_FLAG << 7 | BGC(RED)    | FTC(WHITE) | REVERSE_COLOR_FLAG 
#define DBG_FATAL_COLOR 		HIGHLIGHT_FLAG << 7 | BGC(PURPLE) | FTC(WHITE) 
#define DBG_NON1_COLOR 			HIGHLIGHT_FLAG << 7 | BGC(PURPLE) | FTC(WHITE) | REVERSE_COLOR_FLAG 
#define DBG_SUC_COLOR 			HIGHLIGHT_FLAG << 7 | BGC(GREEN)  | FTC(WHITE)  
#define DBG_CORRECT_COLOR 		HIGHLIGHT_FLAG << 7 | BGC(GREEN)  | FTC(WHITE) | REVERSE_COLOR_FLAG 
#define DBG_NON2_COLOR 			HIGHLIGHT_FLAG << 7 | BGC(YELLOW) | FTC(WHITE)  
#define DBG_WARNNING_COLOR 		HIGHLIGHT_FLAG << 7 | BGC(YELLOW) | FTC(WHITE) | REVERSE_COLOR_FLAG 
#define DBG_NON3_COLOR  		HIGHLIGHT_FLAG << 7 | BGC(BLUE)   | FTC(WHITE)  
#define DBG_FLOW_COLOR  		HIGHLIGHT_FLAG << 7 | BGC(BLUE)   | FTC(WHITE) | REVERSE_COLOR_FLAG 
#define DBG_VIP_COLOR 			HIGHLIGHT_FLAG << 7 | BGC(BLACK)  | FTC(WHITE)  
#define DBG_IMPORTANT_COLOR 	HIGHLIGHT_FLAG << 7 | BGC(BLACK)  | FTC(WHITE) | REVERSE_COLOR_FLAG 
#define DBG_DETAIL_COLOR 							  BGC(BLACK)  | FTC(WHITE) | REVERSE_COLOR_FLAG

#define ERRORIP "cannot find host ip"  


#define OPEN_DEBUG 
#if (defined(ANDROID_USER_MODE))
    #include <android/log.h>
    #ifdef OPEN_DEBUG
    	#define dbg_str(debug_switch,fmt,args...)\
            switch(debug_switch&0xf) {\
                case DBG_PANIC:\
                    __android_log_print(ANDROID_LOG_ERROR, "libobject", fmt, ##args);\
                    break;\
                case DBG_FATAL:\
                    __android_log_print(ANDROID_LOG_WARN, "libobject", fmt, ##args);\
                    break;\
                case DBG_ERROR:\
                    __android_log_print(ANDROID_LOG_WARN, "libobject", fmt, ##args);\
                    break;\
                case DBG_WARNNING:\
                    __android_log_print(ANDROID_LOG_WARN, "libobject", fmt, ##args);\
                    break;\
                case DBG_SUC:\
                    __android_log_print(ANDROID_LOG_INFO, "libobject", fmt, ##args);\
                    break;\
                case DBG_CORRECT:\
                    __android_log_print(ANDROID_LOG_INFO, "libobject", fmt, ##args);\
                    break;\
                case DBG_VIP:\
                    __android_log_print(ANDROID_LOG_INFO, "libobject", fmt, ##args);\
                    break;\
                case DBG_FLOW:\
                    __android_log_print(ANDROID_LOG_DEBUG, "libobject", fmt, ##args);\
                    break;\
                case DBG_IMPORTANT:\
                    __android_log_print(ANDROID_LOG_DEBUG, "libobject", fmt, ##args);\
                    break;\
                case DBG_DETAIL:\
                    __android_log_print(ANDROID_LOG_VERBOSE, "libobject", fmt, ##args);\
                    break;\
                default:\
                    break;\
            }
    	#define dbg_buf(debug_switch,const_str,buf,buf_len) do{}while(0)  
    #else
    	#define dbg_str(debug_switch,fmt,args...) do{}while(0) 
    	#define dbg_buf(debug_switch,const_str,buf,buf_len) do{}while(0)  
    #endif
#else
    #ifdef OPEN_DEBUG
    	#define dbg_str(debug_switch,fmt,args...)\
    		debugger_dbg_str(debugger_gp,debug_switch,"[" fmt "]--[%s:%d]",##args, extract_filename_in_macro(__FILE__), __LINE__);
    	#define dbg_buf(debug_switch,const_str,buf,buf_len)\
    		debugger_dbg_buf(debugger_gp,debug_switch,const_str,buf,buf_len,"[%s:%d]", extract_filename_in_macro(__FILE__), __LINE__) ;
    #else
    	#define dbg_str(debug_switch,fmt,args...) do{}while(0) 
    	#define dbg_buf(debug_switch,const_str,buf,buf_len) do{}while(0)  
    #endif
#endif

#define OPEN_CONSOLE_DEBUG
#ifdef OPEN_CONSOLE_DEBUG
	#define console_str(fmt,args...)\
		printf("[" fmt "]--[%s:%d]\n",##args,__FILE__, __LINE__);
#else
#endif

/*
 *#define OPEN_ATTRIB_PRINT
 */
#ifdef OPEN_ATTRIB_PRINT
    #define ATTRIB_PRINT(fmt,args...) printf(fmt,##args)
#else
    #define ATTRIB_PRINT(fmt,args...) do{}while(0)
#endif

#endif
