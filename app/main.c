#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>

#include "stat_reader.h"
#include "stat_analyzer.h"
#include "stat_printer.h"
#include "buffer_sync.h"

int main()
{
    Buff_sync* reader_buffer = buff_sync_create(10);
    Buff_sync* printer_buffer = buff_sync_create(10);

    Analyzer_buffers* abuffs = abuffs_create(reader_buffer, printer_buffer);

    pthread_t reader, analyzer, printer;
    pthread_create(&reader, NULL, thread_read, (void*)&reader_buffer);
    pthread_create(&analyzer, NULL, thread_analyze, (void*)&abuffs);
    pthread_create(&printer, NULL, thread_print, (void*)&printer_buffer);

    pthread_join(reader, NULL);
    pthread_join(analyzer, NULL);
    pthread_join(printer, NULL);

    buff_sync_destroy(reader_buffer);
    buff_sync_destroy(printer_buffer);

    abuffs_destroy(abuffs);
}