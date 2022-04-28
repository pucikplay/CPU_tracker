#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "buffer_sync.h"

#define STAT_PATH "/proc/stat"
#define LINE_LENGTH 256U

bool reader_starts_with(const char *a, const char *b)
{
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}

size_t reader_read_stat(char** buff, size_t* buff_len, FILE* stat_file)
{
    if (!stat_file || !buff) {
        printf("Reader: invalid buffer or file pointer.\n");
        return 0;
    }
    
    size_t length_needed = 0;
    char line[LINE_LENGTH];


    while (fgets(line, sizeof(line), stat_file)) {
        if (reader_starts_with(line, "cpu")) {
            length_needed += strlen(line);
        } else {
            break;
        }
    }

    fseek(stat_file, 0, SEEK_SET);

    char* new_buff = realloc(*buff, length_needed + 1);

    if (!new_buff) {
        return 0;
    }

    *buff = new_buff;
    *buff_len = length_needed;
    size_t cpu_counter = 0;
    size_t curr_pos = 0;

    while (fgets(line, sizeof(line), stat_file)) {
        if (!reader_starts_with(line, "cpu")) {
            break;
        }
        
        memcpy(*buff + curr_pos, line, strlen(line));
        curr_pos += strlen(line);
        cpu_counter ++;
    }

    (*buff)[curr_pos] = '\0';
    return cpu_counter;
}

void* thread_read(void *arg)
{
    // Buff_sync* bs = *(Buff_sync**)arg;
    
    // pid_t tid = syscall(__NR_gettid);

    // char* buff = 0;
    // size_t buff_size = 0;
    // FILE* stat_file = fopen(STAT_PATH, "r");

    // printf("[%d] Waiting for buffor access\n", tid);
    // buff_sync_lock(bs);

}