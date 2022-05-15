#ifndef STAT_READER_H
#define STAT_READER_H

#include "buffer_sync.h"
#include "stat_control.h"

typedef struct Reader_args Reader_args;

Reader_args* rargs_create(Buff_sync* analyzer_buffer, Buff_sync* logger_buffer, Thread_checkers* work_controller, Thread_stoppers* stop_controller);
void rargs_destroy(Reader_args* rargs);
void* thread_read(void *arg);

#endif
