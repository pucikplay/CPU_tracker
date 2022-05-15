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
#include "stat_watchdog.h"
#include "stat_logger.h"

#define THREADS_NUM 3U
#define LOG_FILE "../log_file.txt"

static Thread_stoppers* stop_controller;

static void term_all_threads(int signum)
{
    if(signum == SIGTERM || signum == SIGINT) {
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
    sigaction(SIGINT, &action, NULL);

    Buff_sync* reader_analyzer_buffer = buff_sync_create(10);
    Buff_sync* analyzer_printer_buffer = buff_sync_create(10);
    Buff_sync* logger_buffer = buff_sync_create(30);

    Reader_args* rargs = rargs_create(reader_analyzer_buffer, work_controller, stop_controller);
    Analyzer_args* aargs = aargs_create(reader_analyzer_buffer, analyzer_printer_buffer, work_controller, stop_controller);
    Printer_args* pargs = pargs_create(analyzer_printer_buffer, work_controller, stop_controller);

    pthread_t reader, analyzer, printer;
    pthread_t watchdog = { 0, };
    pthread_t logger = { 0, };

    pthread_create(&reader, NULL, thread_read, (void*)&rargs);
    pthread_create(&analyzer, NULL, thread_analyze, (void*)&aargs);
    pthread_create(&printer, NULL, thread_print, (void*)&pargs);

    FILE* log_file = fopen(LOG_FILE, "w");
    Logger_args* largs = 0;
    if (log_file) {
        largs = largs_create(logger_buffer, log_file);
    } else {
        largs = largs_create(logger_buffer, stdout);
    }
    pthread_create(&logger, NULL, thread_logger, (void*)&largs);

    pthread_t main_threads[THREADS_NUM] = { reader, analyzer, printer };
    
    Watchdog_args* wargs = wargs_create(work_controller, THREADS_NUM, main_threads);

    pthread_create(&watchdog, NULL, thread_watchdog, (void*)&wargs);

    pthread_join(reader, NULL);
    pthread_join(analyzer, NULL);
    pthread_join(printer, NULL);

    pthread_cancel(watchdog);
    pthread_cancel(logger);
    pthread_join(watchdog, NULL);
    pthread_join(logger, NULL);

    buff_sync_destroy(reader_analyzer_buffer);
    buff_sync_destroy(analyzer_printer_buffer);
    buff_sync_destroy(logger_buffer);

    aargs_destroy(aargs);
    rargs_destroy(rargs);
    pargs_destroy(pargs);
    wargs_destroy(wargs);
    largs_destroy(largs);

    tstop_destroy(stop_controller);
    tcheck_destroy(work_controller);
}
