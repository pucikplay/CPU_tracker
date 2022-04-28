#ifndef CIRC_BUFFER_H
#define CIRC_BUFFER_H

#include<stdbool.h>
#include <stddef.h>

typedef struct circ_buff circ_buff;

circ_buff* circ_init(circ_buff* const cb, const size_t max_len);
circ_buff* circ_create(size_t size);
void circ_delete(circ_buff* cb);
bool circ_is_full(circ_buff* cb);
bool circ_is_empty(circ_buff* cb);
bool circ_append(circ_buff* cb, char* new_elem, size_t elem_len);
char* circ_pop_front(circ_buff* cb);

#endif