#ifndef BUFFER_SYNC_H
#define BUFFER_SYNC_H

#include <stddef.h>
#include <stdbool.h>

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