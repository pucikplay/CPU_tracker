#include "stat_printer.h"
#include "buffer_sync.h"
#include "stat_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define FULL_BAR_LEN 66.0
#define CLEAR_STDOUT() printf("\033[1;1H\033[2J")

struct Printer_args
{
    Buff_sync* analyzer_buffer;
    Thread_checkers* work_controller;
    Thread_stoppers* stop_controller;
};

Printer_args* pargs_create(Buff_sync* analyzer_buffer, Thread_checkers* work_controller, Thread_stoppers* stop_controller)
{
    if (!analyzer_buffer || !work_controller || !stop_controller)
        return 0;

    Printer_args* pargs = malloc(sizeof(*pargs));

    if (!pargs)
        return 0;

    *pargs = (Printer_args){
        .analyzer_buffer = analyzer_buffer,
        .work_controller = work_controller,
        .stop_controller = stop_controller,
    };

    return pargs;
}

void pargs_destroy(Printer_args* pargs)
{
    free(pargs);
}

static void printer_print(char* raw_data)
{
    size_t core_count = 0;
    char** data_tokenized = analyzer_string_split(raw_data, ' ', &core_count);

    CLEAR_STDOUT();

    printf("\t\t\t25%%\t\t50%%\t\t75%%\t\t100%%\n");
    
    printf("CPU:\t");

    size_t bar_length = (size_t)(FULL_BAR_LEN * (atof(data_tokenized[0]) / 100.0));

    for (size_t i = 0; i < bar_length; ++i) {
        putchar('=');
    }
    putchar('\n');

    for (size_t i = 1; i < core_count; ++i) {
        printf("CPU %ld:\t", i - 1);

        bar_length = (size_t)(FULL_BAR_LEN * (atof(data_tokenized[i]) / 100.0));

        for (size_t j = 0; j < bar_length; ++j) {
            putchar('=');
        }
        putchar('\n');
    }

    for (size_t i = 0; i < core_count; ++i) {
        free(data_tokenized[i]);
    }
}

static void printer_buffer_cleanup(void* arg)
{
    if (!arg)
        return;

    char** buffer_to_clean = (char**) arg;
    free(*buffer_to_clean);
}

void* thread_print(void *arg)
{
    Printer_args* pargs = *(Printer_args**)arg;

    Buff_sync* bs = pargs->analyzer_buffer;

    volatile sig_atomic_t* done = tstop_get_analyzer(pargs->stop_controller);
    tcheck_printer_activate(pargs->work_controller);

    char* cpu_data = 0;

    pthread_cleanup_push(printer_buffer_cleanup, &cpu_data)

    while (!*done) {
        tcheck_printer_activate(pargs->work_controller);

        buff_sync_lock(bs);
        
        if (buff_sync_is_empty(bs))
            buff_sync_wait_for_producer(bs);

        cpu_data = buff_sync_pop(bs);

        buff_sync_call_producer(bs);
        buff_sync_unlock(bs);

        if (!cpu_data)
            continue;
        
        printer_print(cpu_data);

        free(cpu_data);
        cpu_data = 0;
    }

    pthread_cleanup_pop(1);

    return NULL;
}
