/*
 * =====================================================================================
 *
 *       Filename:  sync_lock.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  08/24/2015 02:09:15 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef __SYNC_LOCK_H__
#define __SYNC_LOCK_H__

#include <pthread.h>
#include <libobject/user_mode.h>
#include <libobject/basic_types.h>

enum sync_lock_type{
	PTHREAD_MUTEX_LOCK = 1,
	PTHREAD_RWLOCK,
	WINDOWS_MUTEX_LOCK,
	SYNC_LOCK_TYPE_MAX_NUM,
};
typedef struct sync_lock_s{
	uint8_t lock_type;
	struct sync_lock_operations *lock_ops;
	union lock{
		pthread_mutex_t mutex;
		pthread_rwlock_t rwlock;
//#if defined(WINDOWS_USER_MODE)
//        CRITIACAL_SECTION cs;
//#endif
		int b;
	}lock;
}sync_lock_t;
struct sync_lock_operations{
	int (*sync_lock_init)(struct sync_lock_s *slock);
	int (*sync_lock)(struct sync_lock_s *slock,void *arg);
	int (*sync_trylock)(struct sync_lock_s *slock,void *arg);
	int (*sync_unlock)(struct sync_lock_s *slock);
	int (*sync_lock_destroy)(struct sync_lock_s *slock);

};
typedef struct sync_lock_module{
#	define SYNC_LOCK_NAME_MAX_LEN 20
	char name[SYNC_LOCK_NAME_MAX_LEN];
	uint8_t sync_lock_type;
	struct sync_lock_operations sl_ops;
#	undef SYNC_LOCK_NAME_MAX_LEN
}sync_lock_module_t;

extern sync_lock_module_t sync_lock_modules[SYNC_LOCK_TYPE_MAX_NUM];
extern int sync_lock_init(struct sync_lock_s *slock,uint32_t sync_lock_type);

static inline int sync_lock(struct sync_lock_s *slock,void *arg)
{
	if(slock->lock_ops == NULL){
		return 1;
	}
	return slock->lock_ops->sync_lock(slock,arg);
}
static inline int sync_trylock(struct sync_lock_s *slock,void *arg)
{
	if(slock->lock_ops == NULL){
		return 1;
	}
	return slock->lock_ops->sync_trylock(slock,arg);
}
static inline int sync_unlock(struct sync_lock_s *slock)
{
	if(slock->lock_ops == NULL){
		return 1;
	}
	return slock->lock_ops->sync_unlock(slock);
}
static inline int sync_lock_destroy(struct sync_lock_s *slock)
{
	if(slock->lock_ops == NULL){
		return 1;
	}
	return slock->lock_ops->sync_lock_destroy(slock);
}

#endif

