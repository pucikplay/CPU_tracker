#ifndef STAT_READER_H
#define STAT_READER_H

#include "buffer_sync.h"

typedef struct Reader_args Reader_args;

Reader_args* rargs_create(Buff_sync* analyzer_buffer);
void rargs_destroy(Reader_args* rargs);
void* thread_read(void *arg);

#endif
