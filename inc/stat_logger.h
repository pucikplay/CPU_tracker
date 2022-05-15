#ifndef STAT_LOGGER_H
#define STAT_LOGGER_H

#include "buffer_sync.h"

#include <stdio.h>

typedef struct Logger_args Logger_args;

Logger_args* largs_create(Buff_sync* logger_buffer, FILE* log_file);
void largs_destroy(Logger_args* largs);
void* thread_logger(void* arg);

#endif //STAT_LOGGER_H