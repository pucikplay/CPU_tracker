#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "stat_analyzer.h"
#include "buffer_sync.h"
#include "stat_utils.h"

#define NUM_BASE 10

struct Analyzer_buffers
{
    Buff_sync* reader_buffer;
    Buff_sync* printer_buffer;
};

Analyzer_buffers* abuffs_create(Buff_sync* reader_buffer, Buff_sync* printer_buffer)
{
    if (!reader_buffer || !printer_buffer)
        return 0;

    Analyzer_buffers* abuffs = malloc(sizeof(Analyzer_buffers));

    if (!abuffs)
        return 0;

    *abuffs = (Analyzer_buffers){
        .reader_buffer = reader_buffer,
        .printer_buffer = printer_buffer
    };

    return abuffs;
}

void abuffs_destroy(Analyzer_buffers* abuffs)
{
    free(abuffs);
}

static char* analyzer_calc(char* prev_data, char* curr_data)
{
    if (!prev_data || !curr_data)
        return 0;

    size_t core_count = 0;
    char** prev_data_tokenized = analyzer_string_split(prev_data, '\n', &core_count);
    char** curr_data_tokenized = analyzer_string_split(curr_data, '\n', &core_count);

    if (!prev_data_tokenized || !curr_data_tokenized)
        return 0;
    
    if (core_count == 0)
        return 0;

    char* cpu_data = strdup("");
    double cpu_percentage = 0.0;
    char small_buff[12] = { 0, };
    
    for (size_t i = 0; i < core_count; i++) {
        size_t number_count = 0;
        char** prev_line = analyzer_string_split(prev_data_tokenized[i], ' ', &number_count);
        char** curr_line = analyzer_string_split(curr_data_tokenized[i], ' ', &number_count);

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

        free(prev_line);
        free(curr_line);
    }

    for (size_t i = 0; i < core_count; ++i) {
        free(prev_data_tokenized[i]);
        free(curr_data_tokenized[i]); 
    }

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
    Analyzer_buffers* buffs = *(Analyzer_buffers**)arg;

    Buff_sync* rb = buffs->reader_buffer;
    Buff_sync* pb = buffs->printer_buffer;

    bool done = false;
    char* prev_data = 0;
    char* curr_data = 0;
    char* temp_data = 0;
    char* result_data = 0;

    pthread_cleanup_push(analyzer_buffer_cleanup, &prev_data)
    pthread_cleanup_push(analyzer_buffer_cleanup, &curr_data)
    pthread_cleanup_push(analyzer_buffer_cleanup, &temp_data)
    pthread_cleanup_push(analyzer_buffer_cleanup, &result_data)

    while(!prev_data) {
        buff_sync_lock(rb);
        
        if (buff_sync_is_empty(rb))
            buff_sync_wait_for_producer(rb);

        prev_data = buff_sync_pop(rb);

        buff_sync_call_producer(rb);
        buff_sync_unlock(rb);
    }

    while(!done) {
        //receive from reader
        buff_sync_lock(rb);
        
        if (buff_sync_is_empty(rb))
            buff_sync_wait_for_producer(rb);

        curr_data = buff_sync_pop(rb);

        buff_sync_call_producer(rb);
        buff_sync_unlock(rb);

        if (!curr_data)
            continue;

        temp_data = strdup(curr_data);

        result_data = analyzer_calc(prev_data, curr_data);
        //printf("%s\n", result_data);

        //send to printer
        buff_sync_lock(pb);
            
        if (buff_sync_is_full(pb))
            buff_sync_wait_for_consumer(pb);

        buff_sync_append(pb, result_data, strlen(result_data));

        buff_sync_call_consumer(pb);
        buff_sync_unlock(pb);

        //clean
        free(curr_data);
        curr_data = 0;
        free(prev_data);
        prev_data = strdup(temp_data);
        free(temp_data);
        temp_data = 0;

        free(result_data);
        result_data = 0;
        sleep(1);
    }

    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);

    return NULL;
}
