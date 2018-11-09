#ifndef __MESSAGE_MSG_H__
#define __MESSAGE_MSG_H__

#include <stdio.h>
#include <libobject/core/utils/dbg/debug.h>
#include <libobject/core/obj.h>

#define DEFAULT_RAW_MESSAGE_LEN 10

typedef struct message_s{
    void *publisher;
    char *what;
    void *opaque;
    allocator_t *allocator;
} message_t;

message_t *message_alloc(allocator_t *allocator);
int message_set(message_t *message, char *attrib, void *value);

#endif
