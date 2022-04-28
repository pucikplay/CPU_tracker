#ifndef STAT_READER_H
#define STAT_READER_H

#include <stdlib.h>
#include <stdio.h>

size_t reader_read_stat(char** buff, size_t* buff_len, FILE* stat_file);
void* thread_read(void *arg);

#endif