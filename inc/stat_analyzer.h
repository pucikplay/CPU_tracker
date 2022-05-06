#ifndef STAT_ANALYZER_H
#define STAT_ANALYZER_H

#include "buffer_sync.h"

typedef struct Analyzer_buffers Analyzer_buffers;

Analyzer_buffers* abuffs_create(Buff_sync* reader_buffer, Buff_sync* printer_buffer);
void abuffs_destroy(Analyzer_buffers* abuffs);
void* thread_analyze(void *arg);

#endif
