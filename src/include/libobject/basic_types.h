#ifndef __BASIC_TYPES_H__
#define __BASIC_TYPES_H__

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#if (defined(WINDOWS_USER_MODE))
typedef unsigned long long uint64_t;
#endif

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;

#endif
