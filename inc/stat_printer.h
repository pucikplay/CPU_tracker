#ifndef STAT_PRINTER_H
#define STAT_PRINTER_H

#include "buffer_sync.h"

typedef struct Printer_args Printer_args;

Printer_args* pargs_create(Buff_sync* analyzer_buffer);
void pargs_destroy(Printer_args* pargs);
void* thread_print(void *arg);

#endif
