#ifndef STAT_WATCHDOG_H
#define STAT_WATCHDOG_H

#include "stat_control.h"

typedef struct Watchdog_args Watchdog_args;

Watchdog_args* wargs_create(Thread_checkers* work_controller, size_t thread_num, pthread_t threads[]);
void wargs_destroy(Watchdog_args* wargs);
void* thread_watchdog(void* arg);

#endif //STAT_WATCHDOG_H
