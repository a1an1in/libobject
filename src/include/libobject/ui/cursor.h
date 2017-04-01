#ifndef __CURSOR_H__
#define __CURSOR_H__

#include <basic_types.h>

typedef struct cursor_s{
	uint32_t x;
	uint32_t y;
	uint32_t height;
	uint32_t width;
	uint32_t offset;
	char c;
}cursor_t;

#endif
