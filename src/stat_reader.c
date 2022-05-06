#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "buffer_sync.h"
#include "stat_utils.h"
#include "stat_reader.h"

#define STAT_PATH "/proc/stat"
#define LINE_LENGTH 256U

static size_t reader_read_stat(char** buff, size_t* buff_len, FILE* stat_file)
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

    if (!new_buff)
        return 0;

    *buff = new_buff;
    *buff_len = length_needed;
    size_t cpu_counter = 0;
    size_t curr_pos = 0;

    while (fgets(line, sizeof(line), stat_file)) {
        if (!reader_starts_with(line, "cpu"))
            break;
        
        memcpy(*buff + curr_pos, line, strlen(line));
        curr_pos += strlen(line);
        cpu_counter ++;
    }

    (*buff)[curr_pos] = '\0';
    return cpu_counter;
}

static void reader_buffer_cleanup(void* arg)
{
    if (!arg)
        return;

    char** buffer_to_clean = (char**) arg;
    free(*buffer_to_clean);
}

static void reader_file_cleanup(void* arg)
{
    if (!arg)
        return;

    FILE** file_to_clean = (FILE**) arg;
    if (!*file_to_clean)
        return;

    fclose(*file_to_clean);
}

void* thread_read(void *arg)
{
    Buff_sync* bs = *(Buff_sync**)arg;

    char* buff = 0;
    size_t buff_size = 0;
    FILE* stat_file;
    bool done = false;

    pthread_cleanup_push(reader_buffer_cleanup, &buff)
    pthread_cleanup_push(reader_file_cleanup, &stat_file)

    while (!done) {
        stat_file = fopen(STAT_PATH, "r");

        if (stat_file && reader_read_stat(&buff, &buff_size, stat_file)) {
            buff_sync_lock(bs);
            
            if (buff_sync_is_full(bs))
                buff_sync_wait_for_consumer(bs);

            buff_sync_append(bs, buff, buff_size);

            buff_sync_call_consumer(bs);
            buff_sync_unlock(bs);
        }
        
        fclose(stat_file);
        sleep(1);
    }

    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);

    return NULL;

}
