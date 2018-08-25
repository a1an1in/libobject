#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include <libobject/basic_types.h>

enum rw_flag_e{
    RW_FLAG_INIT = 0,
    RW_FLAG_READ,
    RW_FLAG_WRITE,
};
typedef struct ring_buffer_s{
    char *buffer_addr;
    uint32_t buffer_len;
    uint32_t write_index;
    uint32_t read_index;
    uint8_t rw_flag;
}ring_buffer_t;
typedef struct rbuf_msg_head_s{
    uint32_t len;
}rbuf_msg_head_t;

#endif
