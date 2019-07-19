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
	MAX_DEBUG_BUSINESS_NUM
};
extern char *debug_business_names[MAX_DEBUG_BUSINESS_NUM];

#define ALLOC_DBG_PANIC			 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_DBG_PANIC 
#define ALLOC_FATAL				 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_FATAL 
#define ALLOC_ERROR				 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_ERROR 
#define ALLOC_WARNNING			 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_WARNNING 
#define ALLOC_SUC				 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_SUC 
#define ALLOC_CORRECT			 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_CORRECT 
#define ALLOC_VIP 				 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_VIP 
#define ALLOC_FLOW				 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_FLOW 
#define ALLOC_IMPORTANT			 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_IMPORTANT 
#define ALLOC_DETAIL			 DBG_BUSINESS_ALLOC 	 	<< 8 | DBG_DETAIL 

#define LINKLIST_PANIC		 	 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_DBG_PANIC 
#define LINKLIST_FATAL			 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_FATAL 
#define LINKLIST_ERROR			 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_ERROR 
#define LINKLIST_WARNNING		 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_WARNNING 
#define LINKLIST_SUC			 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_SUC 
#define LINKLIST_CORRECT		 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_CORRECT 
#define LINKLIST_VIP 		  	 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_VIP 
#define LINKLIST_FLOW			 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_FLOW 
#define LINKLIST_IMPORTANT		 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_IMPORTANT 
#define LINKLIST_DETAIL			 DBG_BUSINESS_LINKLIST 	 	<< 8 | DBG_DETAIL 

#define HMAP_PANIC		 		 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_DBG_PANIC 
#define HMAP_FATAL				 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_FATAL 
#define HMAP_ERROR				 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_ERROR 
#define HMAP_WARNNING			 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_WARNNING 
#define HMAP_SUC				 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_SUC 
#define HMAP_CORRECT			 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_CORRECT 
#define HMAP_VIP 		  		 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_VIP 
#define HMAP_FLOW				 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_FLOW 
#define HMAP_IMPORTANT			 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_IMPORTANT 
#define HMAP_DETAIL		 		 DBG_BUSINESS_HASHMAP 	 	<< 8 | DBG_DETAIL 

#define RBTMAP_PANIC		 	 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_DBG_PANIC 
#define RBTMAP_FATAL			 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_FATAL 
#define RBTMAP_ERROR			 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_ERROR 
#define RBTMAP_WARNNING			 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_WARNNING 
#define RBTMAP_SUC				 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_SUC 
#define RBTMAP_CORRECT			 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_CORRECT 
#define RBTMAP_VIP 		  		 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_VIP 
#define RBTMAP_FLOW				 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_FLOW 
#define RBTMAP_IMPORTANT		 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_IMPORTANT 
#define RBTMAP_DETAIL			 DBG_BUSINESS_RBTREEMAP 	<< 8 | DBG_DETAIL 

#define VECTOR_PANIC		 	 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_DBG_PANIC 
#define VECTOR_FATAL			 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_FATAL 
#define VECTOR_ERROR			 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_ERROR 
#define VECTOR_WARNNING			 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_WARNNING 
#define VECTOR_SUC				 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_SUC 
#define VECTOR_CORRECT			 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_CORRECT 
#define VECTOR_VIP 		  		 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_VIP 
#define VECTOR_FLOW				 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_FLOW 
#define VECTOR_IMPORTANT		 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_IMPORTANT 
#define VECTOR_DETAIL		 	 DBG_BUSINESS_VECTOR 	 	<< 8 | DBG_DETAIL 

#define CONCURRENT_PANIC		 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_DBG_PANIC 
#define CONCURRENT_FATAL		 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_FATAL 
#define CONCURRENT_ERROR		 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_ERROR 
#define CONCURRENT_WARNNING		 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_WARNNING 
#define CONCURRENT_SUC			 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_SUC 
#define CONCURRENT_CORRECT		 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_CORRECT 
#define CONCURRENT_VIP 		  	 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_VIP 
#define CONCURRENT_FLOW			 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_FLOW 
#define CONCURRENT_IMPORTANT	 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_IMPORTANT 
#define CONCURRENT_DETAIL		 DBG_BUSINESS_CONCURRENT 	<< 8 | DBG_DETAIL 

#define NET_PANIC				 DBG_BUSINESS_NET		 	<< 8 | DBG_DBG_PANIC 
#define NET_FATAL				 DBG_BUSINESS_NET		 	<< 8 | DBG_FATAL 
#define NET_ERROR				 DBG_BUSINESS_NET		 	<< 8 | DBG_ERROR 
#define NET_WARNNING			 DBG_BUSINESS_NET		 	<< 8 | DBG_WARNNING 
#define NET_SUC					 DBG_BUSINESS_NET		 	<< 8 | DBG_SUC 
#define NET_CORRECT				 DBG_BUSINESS_NET		 	<< 8 | DBG_CORRECT 
#define NET_VIP 				 DBG_BUSINESS_NET		 	<< 8 | DBG_VIP 
#define NET_FLOW				 DBG_BUSINESS_NET		 	<< 8 | DBG_FLOW 
#define NET_IMPORTANT			 DBG_BUSINESS_NET		 	<< 8 | DBG_IMPORTANT 
#define NET_DETAIL				 DBG_BUSINESS_NET		 	<< 8 | DBG_DETAIL 

#define PA_PANIC				 DBG_BUSINESS_PA		 	<< 8 | DBG_DBG_PANIC 
#define PA_FATAL				 DBG_BUSINESS_PA		 	<< 8 | DBG_FATAL 
#define PA_ERROR				 DBG_BUSINESS_PA		 	<< 8 | DBG_ERROR 
#define PA_WARNNING			 	 DBG_BUSINESS_PA		 	<< 8 | DBG_WARNNING 
#define PA_SUC					 DBG_BUSINESS_PA		 	<< 8 | DBG_SUC 
#define PA_CORRECT				 DBG_BUSINESS_PA		 	<< 8 | DBG_CORRECT 
#define PA_VIP 				 	 DBG_BUSINESS_PA		 	<< 8 | DBG_VIP 
#define PA_FLOW				 	 DBG_BUSINESS_PA		 	<< 8 | DBG_FLOW 
#define PA_IMPORTANT			 DBG_BUSINESS_PA		 	<< 8 | DBG_IMPORTANT 
#define PA_DETAIL				 DBG_BUSINESS_PA		 	<< 8 | DBG_DETAIL 

#define ARGS_ARGSNIC			 DBG_BUSINESS_ARGS		 	<< 8 | DBG_DBG_ARGSNIC 
#define ARGS_FATAL				 DBG_BUSINESS_ARGS		 	<< 8 | DBG_FATAL 
#define ARGS_ERROR				 DBG_BUSINESS_ARGS		 	<< 8 | DBG_ERROR 
#define ARGS_WARNNING			 DBG_BUSINESS_ARGS		 	<< 8 | DBG_WARNNING 
#define ARGS_SUC				 DBG_BUSINESS_ARGS		 	<< 8 | DBG_SUC 
#define ARGS_CORRECT			 DBG_BUSINESS_ARGS		 	<< 8 | DBG_CORRECT 
#define ARGS_VIP 				 DBG_BUSINESS_ARGS		 	<< 8 | DBG_VIP 
#define ARGS_FLOW				 DBG_BUSINESS_ARGS		 	<< 8 | DBG_FLOW 
#define ARGS_IMPORTANT			 DBG_BUSINESS_ARGS		 	<< 8 | DBG_IMPORTANT 
#define ARGS_DETAIL				 DBG_BUSINESS_ARGS		 	<< 8 | DBG_DETAIL 

#define SM_ARGSNIC			     DBG_BUSINESS_SM		 	<< 8 | DBG_DBG_ARGSNIC 
#define SM_FATAL				 DBG_BUSINESS_SM		 	<< 8 | DBG_FATAL 
#define SM_ERROR				 DBG_BUSINESS_SM		 	<< 8 | DBG_ERROR 
#define SM_WARNNING			     DBG_BUSINESS_SM		 	<< 8 | DBG_WARNNING 
#define SM_SUC				     DBG_BUSINESS_SM		 	<< 8 | DBG_SUC 
#define SM_CORRECT			     DBG_BUSINESS_SM		 	<< 8 | DBG_CORRECT 
#define SM_VIP 				     DBG_BUSINESS_SM		 	<< 8 | DBG_VIP 
#define SM_FLOW				     DBG_BUSINESS_SM		 	<< 8 | DBG_FLOW 
#define SM_IMPORTANT			 DBG_BUSINESS_SM		 	<< 8 | DBG_IMPORTANT 
#define SM_DETAIL				 DBG_BUSINESS_SM		 	<< 8 | DBG_DETAIL 

#define EV_ARGSNIC			     DBG_BUSINESS_EV		 	<< 8 | DBG_DBG_ARGSNIC 
#define EV_FATAL				 DBG_BUSINESS_EV		 	<< 8 | DBG_FATAL 
#define EV_ERROR				 DBG_BUSINESS_EV		 	<< 8 | DBG_ERROR 
#define EV_WARNNING			     DBG_BUSINESS_EV		 	<< 8 | DBG_WARNNING 
#define EV_SUC				     DBG_BUSINESS_EV		 	<< 8 | DBG_SUC 
#define EV_CORRECT			     DBG_BUSINESS_EV		 	<< 8 | DBG_CORRECT 
#define EV_VIP 				     DBG_BUSINESS_EV		 	<< 8 | DBG_VIP 
#define EV_FLOW				     DBG_BUSINESS_EV		 	<< 8 | DBG_FLOW 
#define EV_IMPORTANT			 DBG_BUSINESS_EV		 	<< 8 | DBG_IMPORTANT 
#define EV_DETAIL				 DBG_BUSINESS_EV		 	<< 8 | DBG_DETAIL 

#define BUS_ARGSNIC			     DBG_BUSINESS_BUS		 	<< 8 | DBG_DBG_ARGSNIC 
#define BUS_FATAL				 DBG_BUSINESS_BUS		 	<< 8 | DBG_FATAL 
#define BUS_ERROR				 DBG_BUSINESS_BUS		 	<< 8 | DBG_ERROR 
#define BUS_WARNNING			 DBG_BUSINESS_BUS		 	<< 8 | DBG_WARNNING 
#define BUS_SUC				     DBG_BUSINESS_BUS		 	<< 8 | DBG_SUC 
#define BUS_CORRECT			     DBG_BUSINESS_BUS		 	<< 8 | DBG_CORRECT 
#define BUS_VIP 				 DBG_BUSINESS_BUS		 	<< 8 | DBG_VIP 
#define BUS_FLOW				 DBG_BUSINESS_BUS		 	<< 8 | DBG_FLOW 
#define BUS_IMPORTANT			 DBG_BUSINESS_BUS		 	<< 8 | DBG_IMPORTANT 
#define BUS_DETAIL				 DBG_BUSINESS_BUS		 	<< 8 | DBG_DETAIL 

#define OBJ_ARGSNIC			     DBG_BUSINESS_OBJ		 	<< 8 | DBG_DBG_ARGSNIC 
#define OBJ_FATAL				 DBG_BUSINESS_OBJ		 	<< 8 | DBG_FATAL 
#define OBJ_ERROR				 DBG_BUSINESS_OBJ		 	<< 8 | DBG_ERROR 
#define OBJ_WARNNING			 DBG_BUSINESS_OBJ		 	<< 8 | DBG_WARNNING 
#define OBJ_SUC				     DBG_BUSINESS_OBJ		 	<< 8 | DBG_SUC 
#define OBJ_CORRECT			     DBG_BUSINESS_OBJ		 	<< 8 | DBG_CORRECT 
#define OBJ_VIP 				 DBG_BUSINESS_OBJ		 	<< 8 | DBG_VIP 
#define OBJ_FLOW				 DBG_BUSINESS_OBJ		 	<< 8 | DBG_FLOW 
#define OBJ_IMPORTANT			 DBG_BUSINESS_OBJ		 	<< 8 | DBG_IMPORTANT 
#define OBJ_DETAIL				 DBG_BUSINESS_OBJ		 	<< 8 | DBG_DETAIL 

#define SDL_INTERFACE_ARGSNIC	 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_DBG_ARGSNIC 
#define SDL_INTERFACE_FATAL		 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_FATAL 
#define SDL_INTERFACE_ERROR		 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_ERROR 
#define SDL_INTERFACE_WARNNING	 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_WARNNING 
#define SDL_INTERFACE_SUC		 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_SUC 
#define SDL_INTERFACE_CORRECT	 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_CORRECT 
#define SDL_INTERFACE_VIP 		 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_VIP 
#define SDL_INTERFACE_FLOW		 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_FLOW 
#define SDL_INTERFACE_IMPORTANT	 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_IMPORTANT 
#define SDL_INTERFACE_DETAIL	 DBG_BUSINESS_SDL_INTERFACE	<< 8 | DBG_DETAIL 

#endif
