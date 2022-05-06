#include "stat_printer.h"
#include "buffer_sync.h"
#include "stat_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define FULL_BAR_LEN 66.0

static void printer_print(char* raw_data)
{
    size_t core_count = 0;
    char** data_tokenized = analyzer_string_split(raw_data, ' ', &core_count);

    //clear console screen
    printf("\033[1;1H\033[2J");

    //print percentage scale
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
    Buff_sync* bs = *(Buff_sync**)arg;

    char* cpu_data = 0;
    bool done = false;

    pthread_cleanup_push(printer_buffer_cleanup, &cpu_data)

    while (!done) {
        //receive from anlyzer
        buff_sync_lock(bs);
        
        if (buff_sync_is_empty(bs))
            buff_sync_wait_for_producer(bs);

        cpu_data = buff_sync_pop(bs);

        buff_sync_call_producer(bs);
        buff_sync_unlock(bs);

        if (!cpu_data)
            continue;
        
        //print the data
        printer_print(cpu_data);

        //clean
        free(cpu_data);
        cpu_data = 0;
        sleep(1);
    }

    pthread_cleanup_pop(1);

    return NULL;
}
