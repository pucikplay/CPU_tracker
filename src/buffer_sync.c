#include "buffer_sync.h"
#include "circ_buffer.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>


typedef struct Buff_sync
{
    circ_buff buffer;
    pthread_mutex_t mutex;
    pthread_cond_t can_append;
    pthread_cond_t can_pop;
} Buff_sync;

static Buff_sync* buff_sync_create(size_t max_len)
{
    Buff_sync* new_bs = 0;

    if (!max_len) {
        return 0;
    }

    new_bs = malloc(sizeof(Buff_sync));

    if (!new_bs) {
        return 0;
    }

    *new_bs = (Buff_sync){
                .buffer = circ_create(max_len),
                .mutex = PTHREAD_MUTEX_INITIALIZER,
                .can_append = PTHREAD_COND_INITIALIZER,
                .can_pop = PTHREAD_COND_INITIALIZER,
    }

    if(!new_bs->buffer) {
        free(new_bs);
        new_bs = 0;
        return 0;
    }

    return new_bs;
    
}

static void buff_sync_destroy(Buff_sync* bs)
{
    circ_delete(bs->buffer);
    pthread_mutex_destroy(&bs->mutex);
    pthread_cond_destroy(&bs->can_append);
    pthread_cond_destroy(&bs->can_pop);

    free(bs);
}

static bool buff_sync_is_full(const Buff_sync* bs)
{
    return bs ? circ_is_full(bs->buff) : false;
}

static bool buff_sync_is_empty(const Buff_sync* bs)
{
    return bs ? circ_is_empty(bs->buff) : true;
}

static bool buff_sync_append(Buff_sync* bs, char* new_elem, size_t elem_len)
{
    return bs ? circ_append(bs->buffer, new_elem, elem_len) : false;
}

static char* buff_sync_pop(Buff_sync* bs)
{
    return bs ? circ_pop_front(bs->buffer) : 0;
}


static void buff_sync_lock(Buff_sync* bs)
{
    pthread_mutex_lock(&bs->mutex);
}

static void buff_sync_unlock(Buff_sync* bs)
{
    pthread_mutex_unlock(&bs->mutex);
}

static void buff_sync_call_analyzer(Buff_sync* bs)
{
    pthread_cond_signal(&bs->can_pop);
}

static void buff_sync_call_reader(Buff_sync* bs)
{
    pthread_cond_signal(&bs->can_append);
}

static void buff_sync_wait_for_reader(Buff_sync* bs)
{
    pthread_cond_wait(&bs->can_append, &bs->mutex);
}

static void buff_sync_wait_for_analyzer(Buff_sync* bs)
{
    pthread_cond_wait(&bs->can_pop, &bs->mutex);
}
