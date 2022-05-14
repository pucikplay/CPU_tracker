#ifndef BUFFER_SYNC_H
#define BUFFER_SYNC_H

#include <stddef.h>
#include <stdbool.h>

#define BUFFSYNC_APPEND_STRING(buff_sync, data)         \
do {                                                    \
    buff_sync_lock(buff_sync);                          \
    if (buff_sync_is_full(buff_sync)) {                 \
        buff_sync_wait_for_consumer(buff_sync);         \
    }                                                   \
    buff_sync_append(buff_sync, data, strlen(data));    \
    buff_sync_call_consumer(buff_sync);                 \
    buff_sync_unlock(buff_sync);                        \
} while(false)

#define BUFFSYNC_POP_STRING(buff_sync, target)  \
do {                                            \
    buff_sync_lock(buff_sync);                  \
    if (buff_sync_is_empty(buff_sync)) {        \
        buff_sync_wait_for_producer(buff_sync); \
    }                                           \
    target = buff_sync_pop(buff_sync);          \
    buff_sync_call_producer(buff_sync);         \
    buff_sync_unlock(buff_sync);                \
} while(false)

typedef struct Buff_sync Buff_sync;

Buff_sync* buff_sync_create(size_t max_len);
void buff_sync_destroy(Buff_sync* bs);
bool buff_sync_append(Buff_sync* bs, char* new_elem, size_t elem_len);
char* buff_sync_pop(Buff_sync* bs);
bool buff_sync_is_full(const Buff_sync* bs);
bool buff_sync_is_empty(const Buff_sync* bs);
void buff_sync_lock(Buff_sync* bs);
void buff_sync_unlock(Buff_sync* bs);
void buff_sync_call_consumer(Buff_sync* bs);
void buff_sync_call_producer(Buff_sync* bs);
void buff_sync_wait_for_producer(Buff_sync* bs);
void buff_sync_wait_for_consumer(Buff_sync* bs);

#endif
