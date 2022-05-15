#include "stat_logger.h"
#include "buffer_sync.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

struct Logger_args {
    Buff_sync* logger_buffer;
    FILE* log_file;
};

Logger_args* largs_create(Buff_sync* logger_buffer, FILE* log_file)
{
    if (!logger_buffer || !log_file)
        return 0;

    Logger_args* largs = malloc(sizeof(*largs));

    if (!largs)
        return 0;

    *largs = (Logger_args) {
        .logger_buffer = logger_buffer,
        .log_file = log_file,
    };

    return largs;
}

void largs_destroy(Logger_args* largs)
{
    free(largs);
}

static void logger_buffer_cleanup(void* arg)
{
    if (!arg)
        return;

    char** buffer_to_clean = (char**) arg;
    free(*buffer_to_clean);
}

void* thread_logger(void* arg)
{
    char* log = 0;

    pthread_cleanup_push(logger_buffer_cleanup, (void*) &log);

    Logger_args* largs = *(Logger_args**)arg;

    Buff_sync* lb = largs->logger_buffer;
    FILE* log_file = largs->log_file;

    while (true) {
        BUFFSYNC_POP_STRING(lb, log);

        fprintf(log_file, "%s\n", log);
        free(log);
        log = NULL;
    }

    pthread_cleanup_pop(1);

    return NULL;
}