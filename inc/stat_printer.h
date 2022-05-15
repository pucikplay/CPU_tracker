#ifndef STAT_PRINTER_H
#define STAT_PRINTER_H

#include "buffer_sync.h"
#include "stat_control.h"

typedef struct Printer_args Printer_args;

Printer_args* pargs_create(Buff_sync* analyzer_buffer, Buff_sync* logger_buffer, Thread_checkers* work_controller, Thread_stoppers* stop_controller);
void pargs_destroy(Printer_args* pargs);
void* thread_print(void *arg);

#endif
