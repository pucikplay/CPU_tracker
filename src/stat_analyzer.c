#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>

#include "stat_analyzer.h"
#include "buffer_sync.h"

char** analyzer_string_split(char* restrict data, const char delimiter, size_t* token_count)
{
    if (!data)
        return 0;

    if (!*token_count) {
        char* symbol = data;

        while (*symbol) {
            if (*symbol == delimiter)
                ++(*token_count);

            ++symbol;
        }
    }

    char** result = malloc(sizeof(*result) * (*token_count));
    if (!result)
        return 0;

    size_t idx = 0;
    char delim[2] = {delimiter, '\0'};
    char* token = strtok(data, delim);
    while (token && idx < *token_count) {
        result[idx] = strdup(token);
        token = strtok(0, delim);
        ++idx;
    }
    *token_count = idx;

    return result;
}

char* util_str_concat(char const* restrict str1, char const* restrict str2) {
    const size_t s1_len = strlen(str1);
    const size_t s2_len = strlen(str2);

    char* res = malloc(s1_len + s2_len + 1);
    if (!res)
        return 0;
    
    memcpy(res, str1, s1_len);
    memcpy(res + s1_len, str2, s2_len);
    res[s1_len + s2_len] = '\0';
    
    return res;
}

char* analyzer_calc(char* prev_data, char* curr_data)
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

        //printf("%s %s %s %s %s %s %s %s\n", prev_line[1], prev_line[2], prev_line[3], prev_line[4], prev_line[5], prev_line[6], prev_line[7], prev_line[8]);
        size_t prev_idle = atoi(prev_line[4]) + atoi(prev_line[5]);
        size_t curr_idle = atoi(curr_line[4]) + atoi(curr_line[5]);

        size_t prev_active = atoi(prev_line[1]) + atoi(prev_line[2]) + atoi(prev_line[3]) + atoi(prev_line[6]) + atoi(prev_line[7]) + atoi(prev_line[8]);
        size_t curr_active = atoi(curr_line[1]) + atoi(curr_line[2]) + atoi(curr_line[3]) + atoi(curr_line[6]) + atoi(curr_line[7]) + atoi(curr_line[8]);

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
    free(prev_data);
    free(curr_data);

    return cpu_data;
}


void* thread_analyze(void *arg)
{
    Buff_sync* bs = *(Buff_sync**)arg;

    //pid_t tid = 1;//syscall(__NR_gettid);
    bool done = false;
    char* prev_data = 0;
    char* curr_data = 0;
    char* temp_data = 0;
    char* result_data = 0;

    while(!prev_data) {
        //printf("[%d] Waiting for buffer access\n", tid);
        buff_sync_lock(bs);
        
        if (buff_sync_is_empty(bs)) {
            //printf("[%d] Buffer empty\n", tid);
            buff_sync_wait_for_reader(bs);
        }

        //printf("[%d] Getting data from buffer\n", tid);
        prev_data = buff_sync_pop(bs);

        buff_sync_call_reader(bs);
        buff_sync_unlock(bs);
    }

    while(!done) {
        //printf("[%d] Waiting for buffer access\n", tid);
        buff_sync_lock(bs);
        
        if (buff_sync_is_empty(bs)) {
            //printf("[%d] Buffer empty\n", tid);
            buff_sync_wait_for_reader(bs);
        }

        //printf("[%d] Getting data from buffer\n", tid);
        curr_data = buff_sync_pop(bs);

        buff_sync_call_reader(bs);
        buff_sync_unlock(bs);

        if (!curr_data)
            continue;

        temp_data = strdup(curr_data);

        result_data = analyzer_calc(prev_data, curr_data);
        printf("%s\n", result_data);

        //free(curr_data);
        curr_data = 0;

        //free(prev_data);
        prev_data = strdup(temp_data);
        //free(temp_data);
        temp_data = 0;

        //free(result_data);
        result_data = 0;
        sleep(1);
    }

    //free(prev_data);
    //free(curr_data);

    return NULL;
}