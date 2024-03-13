/*
 * =====================================================================================
 *
 *       Filename:  debug_businesses.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  06/25/2015 06:02:18 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef __DEBUG_BUSINESSES_H__
#define __DEBUG_BUSINESSES_H__

enum debug_business_enum{
	DBG_BUSINESS_NORMAL = 0,
	DBG_BUSINESS_ALLOC,
	DBG_BUSINESS_CONTAINER,
	DBG_BUSINESS_LINKLIST,
	DBG_BUSINESS_HASHMAP,
	DBG_BUSINESS_RBTREEMAP,
	DBG_BUSINESS_VECTOR,
	DBG_BUSINESS_CONCURRENT,
	DBG_BUSINESS_NET,
	DBG_BUSINESS_PA,
	DBG_BUSINESS_ARGS,
	DBG_BUSINESS_SM,
	DBG_BUSINESS_EV,
	DBG_BUSINESS_BUS,
	DBG_BUSINESS_OBJ,
	DBG_BUSINESS_SDL_INTERFACE,
	DBG_BUSINESS_STRING,
	DBG_BUSINESS_ARG,
	DBG_BUSINESS_IO,
	DBG_BUSINESS_DB,
	DBG_BUSINESS_COMPRESS,
	DBG_BUSINESS_CRYPTO,
	MAX_DEBUG_BUSINESS_NUM
};
extern char *debug_business_names[MAX_DEBUG_BUSINESS_NUM];

#define ALLOC_PANIC			     DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_PANIC 
#define ALLOC_FATAL				 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_FATAL 
#define ALLOC_ERROR				 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_ERROR 
#define ALLOC_WARN			     DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_WARN 
#define ALLOC_SUC				 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_SUC 
#define ALLOC_VIP 				 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_VIP 
#define ALLOC_INFO				 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_INFO 
#define ALLOC_DETAIL			 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_DETAIL 

#define LINKLIST_PANIC		 	 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_PANIC 
#define LINKLIST_FATAL			 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_FATAL 
#define LINKLIST_ERROR			 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_ERROR 
#define LINKLIST_WARN		     DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_WARN 
#define LINKLIST_SUC			 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_SUC 
#define LINKLIST_VIP 		  	 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_VIP 
#define LINKLIST_INFO			 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_INFO 
#define LINKLIST_DETAIL			 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_DETAIL 

#define HMAP_PANIC		 		 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_PANIC 
#define HMAP_FATAL				 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_FATAL 
#define HMAP_ERROR				 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_ERROR 
#define HMAP_WARN			     DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_WARN 
#define HMAP_SUC				 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_SUC 
#define HMAP_VIP 		  		 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_VIP 
#define HMAP_INFO				 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_INFO 
#define HMAP_DETAIL		 		 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_DETAIL 

#define RBTMAP_PANIC		 	 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_PANIC 
#define RBTMAP_FATAL			 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_FATAL 
#define RBTMAP_ERROR			 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_ERROR 
#define RBTMAP_WARN			     DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_WARN 
#define RBTMAP_SUC				 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_SUC 
#define RBTMAP_VIP 		  		 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_VIP 
#define RBTMAP_INFO				 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_INFO 
#define RBTMAP_DETAIL			 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_DETAIL 

#define VECTOR_PANIC		 	 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_PANIC 
#define VECTOR_FATAL			 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_FATAL 
#define VECTOR_ERROR			 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_ERROR 
#define VECTOR_WARN			     DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_WARN 
#define VECTOR_SUC				 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_SUC 
#define VECTOR_VIP 		  		 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_VIP 
#define VECTOR_INFO				 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_INFO 
#define VECTOR_DETAIL		 	 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_DETAIL 

#define CONCURRENT_PANIC		 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_PANIC 
#define CONCURRENT_FATAL		 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_FATAL 
#define CONCURRENT_ERROR		 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_ERROR 
#define CONCURRENT_WARN		     DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_WARN 
#define CONCURRENT_SUC			 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_SUC 
#define CONCURRENT_VIP 		  	 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_VIP 
#define CONCURRENT_INFO			 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_INFO 
#define CONCURRENT_DETAIL		 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_DETAIL 

#define NET_PANIC				 DBG_BUSINESS_NET		 	<< 8 | DBG_PANIC 
#define NET_FATAL				 DBG_BUSINESS_NET		 	<< 8 | DBG_FATAL 
#define NET_ERROR				 DBG_BUSINESS_NET		 	<< 8 | DBG_ERROR 
#define NET_WARN			     DBG_BUSINESS_NET		 	<< 8 | DBG_WARN 
#define NET_SUC					 DBG_BUSINESS_NET		 	<< 8 | DBG_SUC 
#define NET_VIP 				 DBG_BUSINESS_NET		 	<< 8 | DBG_VIP 
#define NET_INFO				 DBG_BUSINESS_NET		 	<< 8 | DBG_INFO 
#define NET_DETAIL				 DBG_BUSINESS_NET		 	<< 8 | DBG_DETAIL 

#define PA_PANIC				 DBG_BUSINESS_PA		 	<< 8 | DBG_PANIC 
#define PA_FATAL				 DBG_BUSINESS_PA		 	<< 8 | DBG_FATAL 
#define PA_ERROR				 DBG_BUSINESS_PA		 	<< 8 | DBG_ERROR 
#define PA_WARN			 	     DBG_BUSINESS_PA		 	<< 8 | DBG_WARN 
#define PA_SUC					 DBG_BUSINESS_PA		 	<< 8 | DBG_SUC 
#define PA_VIP 				 	 DBG_BUSINESS_PA		 	<< 8 | DBG_VIP 
#define PA_INFO				 	 DBG_BUSINESS_PA		 	<< 8 | DBG_INFO 
#define PA_DETAIL				 DBG_BUSINESS_PA		 	<< 8 | DBG_DETAIL 

#define ARGS_ARGSNIC			 DBG_BUSINESS_ARGS		 	<< 8 | DBG_PANIC 
#define ARGS_FATAL				 DBG_BUSINESS_ARGS		 	<< 8 | DBG_FATAL 
#define ARGS_ERROR				 DBG_BUSINESS_ARGS		 	<< 8 | DBG_ERROR 
#define ARGS_WARN			     DBG_BUSINESS_ARGS		 	<< 8 | DBG_WARN 
#define ARGS_SUC				 DBG_BUSINESS_ARGS		 	<< 8 | DBG_SUC 
#define ARGS_VIP 				 DBG_BUSINESS_ARGS		 	<< 8 | DBG_VIP 
#define ARGS_INFO				 DBG_BUSINESS_ARGS		 	<< 8 | DBG_INFO 
#define ARGS_DETAIL				 DBG_BUSINESS_ARGS		 	<< 8 | DBG_DETAIL 

#define SM_ARGSNIC			     DBG_BUSINESS_SM		 	<< 8 | DBG_PANIC 
#define SM_FATAL				 DBG_BUSINESS_SM		 	<< 8 | DBG_FATAL 
#define SM_ERROR				 DBG_BUSINESS_SM		 	<< 8 | DBG_ERROR 
#define SM_WARN			         DBG_BUSINESS_SM		 	<< 8 | DBG_WARN 
#define SM_SUC				     DBG_BUSINESS_SM		 	<< 8 | DBG_SUC 
#define SM_VIP 				     DBG_BUSINESS_SM		 	<< 8 | DBG_VIP 
#define SM_INFO				     DBG_BUSINESS_SM		 	<< 8 | DBG_INFO 
#define SM_DETAIL				 DBG_BUSINESS_SM		 	<< 8 | DBG_DETAIL 

#define EV_ARGSNIC			     DBG_BUSINESS_EV		 	<< 8 | DBG_PANIC 
#define EV_FATAL				 DBG_BUSINESS_EV		 	<< 8 | DBG_FATAL 
#define EV_ERROR				 DBG_BUSINESS_EV		 	<< 8 | DBG_ERROR 
#define EV_WARN			         DBG_BUSINESS_EV		 	<< 8 | DBG_WARN 
#define EV_SUC				     DBG_BUSINESS_EV		 	<< 8 | DBG_SUC 
#define EV_VIP 				     DBG_BUSINESS_EV		 	<< 8 | DBG_VIP 
#define EV_INFO				     DBG_BUSINESS_EV		 	<< 8 | DBG_INFO 
#define EV_DETAIL				 DBG_BUSINESS_EV		 	<< 8 | DBG_DETAIL 

#define BUS_ARGSNIC			     DBG_BUSINESS_BUS		 	<< 8 | DBG_PANIC 
#define BUS_FATAL				 DBG_BUSINESS_BUS		 	<< 8 | DBG_FATAL 
#define BUS_ERROR				 DBG_BUSINESS_BUS		 	<< 8 | DBG_ERROR 
#define BUS_WARN			     DBG_BUSINESS_BUS		 	<< 8 | DBG_WARN 
#define BUS_SUC				     DBG_BUSINESS_BUS		 	<< 8 | DBG_SUC 
#define BUS_VIP 				 DBG_BUSINESS_BUS		 	<< 8 | DBG_VIP 
#define BUS_INFO				 DBG_BUSINESS_BUS		 	<< 8 | DBG_INFO 
#define BUS_DETAIL				 DBG_BUSINESS_BUS		 	<< 8 | DBG_DETAIL 

#define OBJ_ARGSNIC			     DBG_BUSINESS_OBJ		 	<< 8 | DBG_PANIC 
#define OBJ_FATAL				 DBG_BUSINESS_OBJ		 	<< 8 | DBG_FATAL 
#define OBJ_ERROR				 DBG_BUSINESS_OBJ		 	<< 8 | DBG_ERROR 
#define OBJ_WARN			     DBG_BUSINESS_OBJ		 	<< 8 | DBG_WARN 
#define OBJ_SUC				     DBG_BUSINESS_OBJ		 	<< 8 | DBG_SUC 
#define OBJ_VIP 				 DBG_BUSINESS_OBJ		 	<< 8 | DBG_VIP 
#define OBJ_INFO				 DBG_BUSINESS_OBJ		 	<< 8 | DBG_INFO 
#define OBJ_DETAIL				 DBG_BUSINESS_OBJ		 	<< 8 | DBG_DETAIL 

#define SDL_INTERFACE_ARGSNIC	 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_PANIC 
#define SDL_INTERFACE_FATAL		 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_FATAL 
#define SDL_INTERFACE_ERROR		 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_ERROR 
#define SDL_INTERFACE_WARN	     DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_WARN 
#define SDL_INTERFACE_SUC		 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_SUC 
#define SDL_INTERFACE_VIP 		 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_VIP 
#define SDL_INTERFACE_INFO		 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_INFO 
#define SDL_INTERFACE_DETAIL	 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_DETAIL 

#define STRING_ARGSNIC			 DBG_BUSINESS_STRING		<< 8 | DBG_PANIC 
#define STRING_FATAL			 DBG_BUSINESS_STRING		<< 8 | DBG_FATAL 
#define STRING_ERROR			 DBG_BUSINESS_STRING		<< 8 | DBG_ERROR 
#define STRING_WARN			     DBG_BUSINESS_STRING		<< 8 | DBG_WARN 
#define STRING_SUC				 DBG_BUSINESS_STRING		<< 8 | DBG_SUC 
#define STRING_VIP 				 DBG_BUSINESS_STRING		<< 8 | DBG_VIP 
#define STRING_INFO				 DBG_BUSINESS_STRING		<< 8 | DBG_INFO 
#define STRING_DETAIL			 DBG_BUSINESS_STRING		<< 8 | DBG_DETAIL 

#define ARG_ARGSNIC			     DBG_BUSINESS_ARG		 	<< 8 | DBG_PANIC 
#define ARG_FATAL				 DBG_BUSINESS_ARG		 	<< 8 | DBG_FATAL 
#define ARG_ERROR				 DBG_BUSINESS_ARG		 	<< 8 | DBG_ERROR 
#define ARG_WARN			     DBG_BUSINESS_ARG		 	<< 8 | DBG_WARN 
#define ARG_SUC				     DBG_BUSINESS_ARG		 	<< 8 | DBG_SUC 
#define ARG_VIP 				 DBG_BUSINESS_ARG		 	<< 8 | DBG_VIP 
#define ARG_INFO				 DBG_BUSINESS_ARG		 	<< 8 | DBG_INFO 
#define ARG_DETAIL				 DBG_BUSINESS_ARG		 	<< 8 | DBG_DETAIL 

#define IO_ARGSNIC			     DBG_BUSINESS_IO		 	<< 8 | DBG_PANIC 
#define IO_FATAL				 DBG_BUSINESS_IO		 	<< 8 | DBG_FATAL 
#define IO_ERROR				 DBG_BUSINESS_IO		 	<< 8 | DBG_ERROR 
#define IO_WARN			         DBG_BUSINESS_IO		 	<< 8 | DBG_WARN 
#define IO_SUC				     DBG_BUSINESS_IO		 	<< 8 | DBG_SUC 
#define IO_VIP 				     DBG_BUSINESS_IO		 	<< 8 | DBG_VIP 
#define IO_INFO				     DBG_BUSINESS_IO		 	<< 8 | DBG_INFO 
#define IO_DETAIL				 DBG_BUSINESS_IO		 	<< 8 | DBG_DETAIL 

#define DB_ARGSNIC			     DBG_BUSINESS_DB		 	<< 8 | DBG_PANIC 
#define DB_FATAL				 DBG_BUSINESS_DB		 	<< 8 | DBG_FATAL 
#define DB_ERROR				 DBG_BUSINESS_DB		 	<< 8 | DBG_ERROR 
#define DB_WARN			         DBG_BUSINESS_DB		 	<< 8 | DBG_WARN 
#define DB_SUC				     DBG_BUSINESS_DB		 	<< 8 | DBG_SUC 
#define DB_VIP 				     DBG_BUSINESS_DB		 	<< 8 | DBG_VIP 
#define DB_INFO				     DBG_BUSINESS_DB		 	<< 8 | DBG_INFO 
#define DB_DETAIL				 DBG_BUSINESS_DB		 	<< 8 | DBG_DETAIL 

#define CRYPTO_ARGSNIC			 DBG_BUSINESS_CRYPTO		<< 8 | DBG_PANIC 
#define CRYPTO_FATAL			 DBG_BUSINESS_CRYPTO		<< 8 | DBG_FATAL 
#define CRYPTO_ERROR			 DBG_BUSINESS_CRYPTO		<< 8 | DBG_ERROR 
#define CRYPTO_WARN			     DBG_BUSINESS_CRYPTO		<< 8 | DBG_WARN 
#define CRYPTO_SUC				 DBG_BUSINESS_CRYPTO		<< 8 | DBG_SUC 
#define CRYPTO_VIP 				 DBG_BUSINESS_CRYPTO		<< 8 | DBG_VIP 
#define CRYPTO_INFO				 DBG_BUSINESS_CRYPTO		<< 8 | DBG_INFO 
#define CRYPTO_DETAIL			 DBG_BUSINESS_CRYPTO		<< 8 | DBG_DETAIL 

#define COMPRESS_ARGSNIC		 DBG_BUSINESS_COMPRESS		<< 8 | DBG_PANIC 
#define COMPRESS_FATAL			 DBG_BUSINESS_COMPRESS		<< 8 | DBG_FATAL 
#define COMPRESS_ERROR			 DBG_BUSINESS_COMPRESS		<< 8 | DBG_ERROR 
#define COMPRESS_WARN		     DBG_BUSINESS_COMPRESS		<< 8 | DBG_WARN 
#define COMPRESS_SUC			 DBG_BUSINESS_COMPRESS		<< 8 | DBG_SUC 
#define COMPRESS_VIP 			 DBG_BUSINESS_COMPRESS		<< 8 | DBG_VIP 
#define COMPRESS_INFO			 DBG_BUSINESS_COMPRESS		<< 8 | DBG_INFO 
#define COMPRESS_DETAIL			 DBG_BUSINESS_COMPRESS		<< 8 | DBG_DETAIL 
#endif
