#include <string.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

#include "circ_buffer.h"

struct circ_buff {
    size_t start;
    size_t len;
    size_t max_len;
    char** arr;
};

static circ_buff* circ_init(circ_buff* const cb, const size_t max_len) {
    if (!cb) {
        return 0;
    }

    if (max_len) {
        *cb = (circ_buff) {
            .max_len = max_len,
            .arr = malloc(max_len * sizeof(char*)),
        };
        if (!cb->arr) cb->max_len = 0;
    }
    else {
        *cb = (circ_buff) { 0, };
    }

    return cb;
}

circ_buff* circ_create(size_t max_len) {
    return circ_init(malloc(sizeof(circ_buff)), max_len); 
}

static void circ_remove_arr(circ_buff* cb) {
    size_t i = cb->start;
    size_t counter = 0;
    while (counter < cb->len) {
        free(cb->arr[i]);
        ++i;
        i %= cb->max_len;
        counter ++;
    }
    free(cb->arr);
}

void circ_destroy(circ_buff* cb) {
    if (cb) {
        circ_remove_arr(cb);
    }
    free(cb);
}

bool circ_is_full(circ_buff* cb) {
    return cb ? cb->len == cb->max_len : false;
}

bool circ_is_empty(circ_buff* cb) {
    return cb ? cb->len == 0 : true;
}

static size_t circ_getpos(const circ_buff* cb, size_t pos) {
    pos += cb->start;
    pos %= cb->max_len;
    return pos;
}

static char** circ_getelem(const circ_buff* cb, const size_t pos) {
    if (pos >= cb->max_len) {
        return 0;
    }

    size_t true_pos = circ_getpos(cb, pos);
    return &cb->arr[true_pos];
}

bool circ_append(circ_buff* cb, char* new_elem, size_t elem_len) {
    if (!cb || !new_elem || cb->len >= cb->max_len) {
        return false;
    }

    size_t end_pos = circ_getpos(cb, cb->len);
    cb->arr[end_pos] = malloc(sizeof(**(cb->arr)) * elem_len + 1);

    if(!cb->arr[end_pos]) {
        return false;
    }

    (cb->len)++;
    strncpy(cb->arr[end_pos], new_elem, elem_len);
    (cb->arr[end_pos])[elem_len] = '\0';
    return true;
}

char* circ_pop_front(circ_buff* cb) {
    if (!cb || !cb->len) {
        return 0;
    }

    char** first_elem = circ_getelem(cb, 0);

    char* res = *first_elem;

    *first_elem = 0;
    cb->start = (cb->start + 1) % cb->max_len;
    cb->len--;
    return res;
}
