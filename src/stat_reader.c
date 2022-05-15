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
#include "stat_control.h"

#define STAT_PATH "/proc/stat"
#define LINE_LENGTH 256U
#define READER_SLEEP_TIME 1U

struct Reader_args
{
    Buff_sync* analyzer_buffer;
    Buff_sync* logger_buffer;
    Thread_checkers* work_controller;
    Thread_stoppers* stop_controller;
};

Reader_args* rargs_create(Buff_sync* analyzer_buffer, Buff_sync* logger_buffer, Thread_checkers* work_controller, Thread_stoppers* stop_controller)
{
    if (!analyzer_buffer || !logger_buffer || !work_controller || !stop_controller)
        return 0;

    Reader_args* rargs = malloc(sizeof(*rargs));

    if (!rargs)
        return 0;

    *rargs = (Reader_args){
        .analyzer_buffer = analyzer_buffer,
        .logger_buffer = logger_buffer,
        .work_controller = work_controller,
        .stop_controller = stop_controller,
    };

    return rargs;
}

void rargs_destroy(Reader_args* rargs)
{
    free(rargs);
}

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
    Reader_args* rargs = *(Reader_args**)arg;

    Buff_sync* ab = rargs->analyzer_buffer;
    Buff_sync* lb = rargs->logger_buffer;

    volatile sig_atomic_t* done = tstop_get_analyzer(rargs->stop_controller);
    tcheck_reader_activate(rargs->work_controller);

    char* buff = 0;
    size_t buff_size = 0;
    FILE* stat_file;

    pthread_cleanup_push(reader_buffer_cleanup, &buff)
    pthread_cleanup_push(reader_file_cleanup, &stat_file)

    BUFFSYNC_APPEND_STRING(lb, "Reader thread initialized");

    while (!*done) {
        tcheck_reader_activate(rargs->work_controller);
        sleep(READER_SLEEP_TIME);

        stat_file = fopen(STAT_PATH, "r");

        if (stat_file && reader_read_stat(&buff, &buff_size, stat_file)) {
            BUFFSYNC_APPEND_STRING(ab, buff);
        }

        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
        fclose(stat_file);
        stat_file = NULL;
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    }

    BUFFSYNC_APPEND_STRING(lb, "Reader thread done");

    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);

    return NULL;
}
