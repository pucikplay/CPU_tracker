#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "stat_analyzer.h"
#include "buffer_sync.h"
#include "stat_utils.h"
#include "stat_control.h"

#define NUM_BASE 10

struct Analyzer_args
{
    Buff_sync* reader_buffer;
    Buff_sync* printer_buffer;
    Thread_checkers* work_controller;
    Thread_stoppers* stop_controller;
};

Analyzer_args* aargs_create(Buff_sync* reader_buffer, Buff_sync* printer_buffer, Thread_checkers* work_controller, Thread_stoppers* stop_controller)
{
    if (!reader_buffer || !printer_buffer || !work_controller || !stop_controller)
        return 0;

    Analyzer_args* aargs = malloc(sizeof(*aargs));

    if (!aargs)
        return 0;

    *aargs = (Analyzer_args){
        .reader_buffer = reader_buffer,
        .printer_buffer = printer_buffer,
        .work_controller = work_controller,
        .stop_controller = stop_controller,
    };

    return aargs;
}

void aargs_destroy(Analyzer_args* aargs)
{
    free(aargs);
}

static char* analyzer_calc(char* prev_data, char* curr_data)
{
    if (!prev_data || !curr_data)
        return 0;

    size_t core_count = 0;
    char** prev_data_tokenized = util_str_split(prev_data, '\n', &core_count);
    char** curr_data_tokenized = util_str_split(curr_data, '\n', &core_count);

    if (!prev_data_tokenized || !curr_data_tokenized)
        return 0;
    
    if (core_count == 0)
        return 0;

    char* cpu_data = strdup("");
    double cpu_percentage = 0.0;
    char small_buff[12] = { 0, };
    
    for (size_t i = 0; i < core_count; i++) {
        size_t number_count = 0;
        char** prev_line = util_str_split(prev_data_tokenized[i], ' ', &number_count);
        char** curr_line = util_str_split(curr_data_tokenized[i], ' ', &number_count);

        if (!prev_line || !curr_line)
            continue;

        size_t prev_idle = strtoul(prev_line[4], NULL, NUM_BASE) + \
                            strtoul(prev_line[5], NULL, NUM_BASE);
        size_t curr_idle = strtoul(curr_line[4], NULL, NUM_BASE) + \
                            strtoul(curr_line[5], NULL, NUM_BASE);

        size_t prev_active = strtoul(prev_line[1], NULL, NUM_BASE) + \
                                strtoul(prev_line[2], NULL, NUM_BASE) + \
                                strtoul(prev_line[3], NULL, NUM_BASE) + \
                                strtoul(prev_line[6], NULL, NUM_BASE) + \
                                strtoul(prev_line[7], NULL, NUM_BASE) + \
                                strtoul(prev_line[8], NULL, NUM_BASE);
        size_t curr_active = strtoul(curr_line[1], NULL, NUM_BASE) + \
                                strtoul(curr_line[2], NULL, NUM_BASE) + \
                                strtoul(curr_line[3], NULL, NUM_BASE) + \
                                strtoul(curr_line[6], NULL, NUM_BASE) + \
                                strtoul(curr_line[7], NULL, NUM_BASE) + \
                                strtoul(curr_line[8], NULL, NUM_BASE);

        size_t prev_total = prev_idle + prev_active;
        size_t curr_total = curr_idle + curr_active;

        size_t total_diff = curr_total - prev_total;
        size_t idle_diff = curr_idle - prev_idle;

        cpu_percentage = (double)(total_diff - idle_diff)/(double)total_diff * 1e2;

        sprintf(small_buff, "%.2lf%s ", cpu_percentage, "%");

        char* mid_result_str = util_str_concat(cpu_data, small_buff);

        free(cpu_data);
        cpu_data = mid_result_str;

        util_split_cleanup(prev_line, number_count);
        util_split_cleanup(curr_line, number_count);
    }

    util_split_cleanup(prev_data_tokenized, core_count);
    util_split_cleanup(curr_data_tokenized, core_count);

    return cpu_data;
}

static void analyzer_buffer_cleanup(void* arg)
{
    if (!arg)
        return;

    char** buffer_to_clean = (char**) arg;
    free(*buffer_to_clean);
}


void* thread_analyze(void *arg)
{
    Analyzer_args* aargs = *(Analyzer_args**)arg;

    Buff_sync* rb = aargs->reader_buffer;
    Buff_sync* pb = aargs->printer_buffer;

    volatile sig_atomic_t* done = tstop_get_analyzer(aargs->stop_controller);
    tcheck_analyzer_activate(aargs->work_controller);

    char* prev_data = 0;
    char* curr_data = 0;
    char* temp_data = 0;
    char* result_data = 0;

    pthread_cleanup_push(analyzer_buffer_cleanup, &prev_data)
    pthread_cleanup_push(analyzer_buffer_cleanup, &curr_data)
    pthread_cleanup_push(analyzer_buffer_cleanup, &temp_data)
    pthread_cleanup_push(analyzer_buffer_cleanup, &result_data)

    while(!prev_data) {
        tcheck_analyzer_activate(aargs->work_controller);
        buff_sync_lock(rb);
        
        if (buff_sync_is_empty(rb))
            buff_sync_wait_for_producer(rb);

        prev_data = buff_sync_pop(rb);

        buff_sync_call_producer(rb);
        buff_sync_unlock(rb);
    }

    while(!*done) {
        tcheck_analyzer_activate(aargs->work_controller);

        BUFFSYNC_POP_STRING(rb, curr_data);

        if (!curr_data)
            continue;

        temp_data = strdup(curr_data);

        result_data = analyzer_calc(prev_data, curr_data);

        BUFFSYNC_APPEND_STRING(pb, result_data);

        free(curr_data);
        curr_data = 0;
        free(prev_data);
        prev_data = strdup(temp_data);
        free(temp_data);
        temp_data = 0;

        free(result_data);
        result_data = 0;
    }

    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);

    return NULL;
}
