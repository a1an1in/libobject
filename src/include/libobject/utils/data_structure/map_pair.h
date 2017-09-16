/*
 * =====================================================================================
 *
 *       Filename:  map_pair.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  12/01/2015 07:21:59 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  alan lin (), a1an1in@sina.com
 *   Organization:  
 *
 * =====================================================================================
 */
#include <string.h>
#include <stdlib.h>

#ifndef __MAP_PAIR_H__
#define __MAP_PAIR_H__
typedef struct pair{
	uint16_t data_len;
	uint32_t key_len;
	uint32_t value_len;
	uint8_t data[1];
}pair_t;

#ifndef __KEY_CMP_FPT__
#define __KEY_CMP_FPT__
typedef int (*key_cmp_fpt)(void *key1,void *key2,uint32_t key_size);
#endif
static inline pair_t * create_pair(int key_len,int value_len)
{
	pair_t *p;
	p = (pair_t *)malloc(sizeof(pair_t) + key_len + value_len);
	memset(p,0,sizeof(pair_t) + key_len + value_len);
	p->key_len = key_len;
	p->value_len = value_len;
	p->data_len = key_len + value_len;
	return p;
}
static inline void make_pair(pair_t *p,void *key,void *value)
{
    memset(p->data,0, p->data_len);
    if(strlen(key) >=  p->key_len) {
        memcpy(p->data,key,p->key_len);
    } else {
        memcpy(p->data,key,strlen(key));
    }
	memcpy(p->data + p->key_len,value,p->value_len);
}
static inline int destroy_pair(pair_t *p)
{
	free(p);
	p = NULL;
	return 0;
}


#endif

