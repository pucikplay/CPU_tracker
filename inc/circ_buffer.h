#define CIRC_BUFFER
#ifndef CIRCBUFFER

typedef struct circ_buff circ_buff;

circ_buff* circ_buffer_create(size_t size);

#endif