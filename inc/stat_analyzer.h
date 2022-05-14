#ifndef STAT_ANALYZER_H
#define STAT_ANALYZER_H

#include "buffer_sync.h"

typedef struct Analyzer_args Analyzer_args;

Analyzer_args* aargs_create(Buff_sync* reader_buffer, Buff_sync* printer_buffer);
void aargs_destroy(Analyzer_args* aargs);
void* thread_analyze(void *arg);

#endif
