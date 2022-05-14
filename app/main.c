#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "stat_reader.h"
#include "stat_analyzer.h"
#include "stat_printer.h"
#include "buffer_sync.h"
#include "stat_control.h"

static Thread_stoppers* stop_controller;

static void term_all_threads(int signum)
{
    if(signum == SIGTERM) {
        tstop_stop_threads(stop_controller);
    }
}

int main()
{
    Thread_checkers* work_controller = tcheck_create();
    stop_controller = tstop_create();

    struct sigaction action = { 0, };
    action.sa_handler = term_all_threads;
    sigaction(SIGTERM, &action, NULL);

    Buff_sync* reader_analyzer_buffer = buff_sync_create(10);
    Buff_sync* analyzer_printer_buffer = buff_sync_create(10);

    Reader_args* rargs = rargs_create(reader_analyzer_buffer);
    Analyzer_args* aargs = aargs_create(reader_analyzer_buffer, analyzer_printer_buffer);
    Printer_args* pargs = pargs_create(analyzer_printer_buffer);

    pthread_t reader, analyzer, printer;
    pthread_create(&reader, NULL, thread_read, (void*)&rargs);
    pthread_create(&analyzer, NULL, thread_analyze, (void*)&aargs);
    pthread_create(&printer, NULL, thread_print, (void*)&pargs);

    pthread_join(reader, NULL);
    pthread_join(analyzer, NULL);
    pthread_join(printer, NULL);

    aargs_destroy(aargs);
    rargs_destroy(rargs);
    pargs_destroy(pargs);

    tstop_destroy(stop_controller);
    tcheck_destroy(work_controller);
}
