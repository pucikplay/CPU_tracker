#include <stdio.h>
#include <stddef.h>
#include <unistd.h>
#include <pthread.h>

#include "stat_reader.h"
#include "stat_analyzer.h"
#include "buffer_sync.h"

int main(void) {

    Buff_sync* bs = buff_sync_create(10);

    pthread_t reader, analyzer;
    pthread_create(&reader, NULL, thread_read, (void*)&bs);
    pthread_create(&analyzer, NULL, thread_analyze, (void*)&bs);

    pthread_join(reader, NULL);
    pthread_join(analyzer, NULL);

    buff_sync_destroy(bs);

    return 0;
}