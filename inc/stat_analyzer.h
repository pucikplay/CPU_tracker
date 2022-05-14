#ifndef STAT_ANALYZER_H
#define STAT_ANALYZER_H

#include "buffer_sync.h"
#include "stat_control.h"

typedef struct Analyzer_args Analyzer_args;

Analyzer_args* aargs_create(Buff_sync* reader_buffer, Buff_sync* printer_buffer, Thread_checkers* work_controller, Thread_stoppers* stop_controller);
void aargs_destroy(Analyzer_args* aargs);
void* thread_analyze(void *arg);

#endif
