#ifndef __BYTE_ORDER_H__
#define __BYTE_ORDER_H__

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/utils/registry/registry.h>
#include <libobject/basic_types.h>

#if (defined(WINDOWS_USER_MODE))
typedef unsigned long long uint64_t;
#endif

#define ___constant_swab16(x) ((uint16_t)(				             \
	(((uint16_t)(x) & (uint16_t)0x00ffU) << 8) |			         \
	(((uint16_t)(x) & (uint16_t)0xff00U) >> 8)))

#define ___constant_swab32(x) ((uint32_t)(				             \
	(((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) |		         \
	(((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) |		         \
	(((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) |		         \
	(((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24)))

#define ___constant_swab64(x) ((uint64_t)(				             \
	(((uint64_t)(x) & (uint64_t)0x00000000000000ffULL) << 56) |	     \
	(((uint64_t)(x) & (uint64_t)0x000000000000ff00ULL) << 40) |	     \
	(((uint64_t)(x) & (uint64_t)0x0000000000ff0000ULL) << 24) |	     \
	(((uint64_t)(x) & (uint64_t)0x00000000ff000000ULL) <<  8) |	     \
	(((uint64_t)(x) & (uint64_t)0x000000ff00000000ULL) >>  8) |	     \
	(((uint64_t)(x) & (uint64_t)0x0000ff0000000000ULL) >> 24) |	     \
	(((uint64_t)(x) & (uint64_t)0x00ff000000000000ULL) >> 40) |	     \
	(((uint64_t)(x) & (uint64_t)0xff00000000000000ULL) >> 56)))


#define LE32_TO_CPU(value) ((value) = byteorder_le32_to_cpu(&(value)))
#define LE16_TO_CPU(value) ((value) = byteorder_le16_to_cpu(&(value)))
#define CPU_TO_LE32(value) ((value) = byteorder_cpu_to_le32(&(value)))
#define CPU_TO_LE16(value) ((value) = byteorder_cpu_to_le16(&(value)))

static inline int byteorder_is_cpu_little_endian()
{
    union w {  
        int a;
        char b;
    } c;
    c.a = 1;
    return(c.b ==1);
}

static inline uint64_t byteorder_be64_to_cpu(uint64_t *p)
{
    if (byteorder_is_cpu_little_endian()) {
        return ___constant_swab64((*p));
    }

    return *p;
}

static inline uint64_t byteorder_le64_to_cpu(uint64_t *p)
{
    if (!byteorder_is_cpu_little_endian()) {
        return ___constant_swab64((*p));
    }

    return *p;
}

static inline uint64_t byteorder_cpu_to_be64(uint64_t *p)
{
    if (byteorder_is_cpu_little_endian()) {
        return ___constant_swab64((*p));
    }

    return *p;
}

static inline uint64_t byteorder_cpu_to_le64(uint64_t *p)
{
    if (!byteorder_is_cpu_little_endian()) {
        return ___constant_swab64((*p));
    }

    return *p;
}


static inline uint32_t byteorder_be32_to_cpu(uint32_t *p)
{
    if (byteorder_is_cpu_little_endian()) {
        return ___constant_swab32((*p));
    }

    return *p;
}

static inline uint32_t byteorder_le32_to_cpu(uint32_t *p)
{
    if (!byteorder_is_cpu_little_endian()) {
        return ___constant_swab32((*p));
    }

    return *p;
}

static inline uint32_t byteorder_cpu_to_be32(uint32_t *p)
{
    if (byteorder_is_cpu_little_endian()) {
        return ___constant_swab32((*p));
    }

    return *p;
}

static inline uint32_t byteorder_cpu_to_le32(uint32_t *p)
{
    if (!byteorder_is_cpu_little_endian()) {
        return ___constant_swab32((*p));
    }

    return *p;
}

static inline uint16_t byteorder_be16_to_cpu(uint16_t *p)
{
    if (byteorder_is_cpu_little_endian()) {
        return ___constant_swab16((*p));
    }

    return *p;
}

static inline uint16_t byteorder_le16_to_cpu(uint16_t *p)
{
    if (!byteorder_is_cpu_little_endian()) {
        return ___constant_swab16((*p));
    }

    return *p;
}

static inline uint16_t byteorder_cpu_to_be16(uint16_t *p)
{
    if (byteorder_is_cpu_little_endian()) {
        return ___constant_swab16((*p));
    }

    return *p;
}

static inline uint16_t byteorder_cpu_to_le16(uint16_t *p)
{
    if (!byteorder_is_cpu_little_endian()) {
        return ___constant_swab16((*p));
    }

    return *p;
}

#endif