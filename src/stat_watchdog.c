#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "stat_watchdog.h"
#include "stat_control.h"

#define WATCHDOG_SLEEP_TIME 2U

struct Watchdog_args {
    Thread_checkers* work_controller;
    size_t thread_num;
    pthread_t threads[];
};

Watchdog_args* wargs_create(Thread_checkers* work_controller, size_t thread_num, pthread_t threads[])
{
    if (!work_controller || !thread_num || !threads)
        return 0;

    Watchdog_args* wargs = malloc(sizeof(*wargs) + sizeof(pthread_t) * thread_num);
    if (!wargs)
        return 0;

    wargs->work_controller = work_controller;
    wargs->thread_num = thread_num;
    memcpy(wargs->threads, threads, sizeof(pthread_t) * thread_num);

    return wargs;
}

void wargs_destroy(Watchdog_args* wargs)
{
    free(wargs);
}

void* thread_watchdog(void* arg)
{
    Watchdog_args* wargs = *(Watchdog_args**)arg;

    size_t thread_num = wargs->thread_num;
    pthread_t* threads = wargs->threads;
    Thread_checkers* work_controller = wargs->work_controller;

    while (true) {
        sleep(WATCHDOG_SLEEP_TIME);
        if (!tcheck_check(work_controller))
            break;

        tcheck_reset(work_controller);
    }

    //This part is only reached if tcheck_check() fails
    for (size_t i = 0; i < thread_num; i++) {
        if (threads[i])
            pthread_cancel(threads[i]);
    }

    return NULL;
}
