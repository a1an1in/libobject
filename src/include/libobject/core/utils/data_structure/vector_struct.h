#ifndef __VECTOR_STRUCT_H__
#define __VECTOR_STRUCT_H__

#include <libobject/core/utils/alloc/allocator.h>
#include <libobject/core/utils/thread/sync_lock.h>

typedef struct vector_pos_s{
	struct vector_s *vector;
	void *vector_head;
	uint32_t vector_pos;
}vector_pos_t;

typedef struct vector_s{
	uint32_t data_size;
	uint32_t capacity;
	uint32_t step;
	uint32_t size;
	void **vector_head;
	sync_lock_t vector_lock;
	uint8_t lock_type;
	allocator_t *allocator;
	vector_pos_t begin,end;
}vector_t;

#endif
