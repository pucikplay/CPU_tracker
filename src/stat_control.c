#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdatomic.h>

#include "stat_control.h"

struct Thread_stoppers {
    volatile sig_atomic_t reader_done;
    volatile sig_atomic_t analyzer_done;
    volatile sig_atomic_t printer_done;
};

Thread_stoppers* tstop_create()
{
    Thread_stoppers* tstop = malloc(sizeof(*tstop));
    if (!tstop)
        return 0;

    *tstop = (Thread_stoppers) {
        .reader_done = 0,
        .analyzer_done = 0,
        .printer_done = 0,
    };

    return tstop;
}

void tstop_destroy(Thread_stoppers* tstop) 
{
    free(tstop);
}

void tstop_stop_threads(Thread_stoppers* tstop)
{
    if (!tstop)
        return;

    tstop->reader_done = 1;
    tstop->analyzer_done = 1;
    tstop->printer_done = 1;
}

sig_atomic_t volatile* tstop_get_reader(Thread_stoppers* tstop)
{
    return tstop ? &tstop->reader_done : 0;
}

sig_atomic_t volatile* tstop_get_analyzer(Thread_stoppers* tstop)
{
    return tstop ? &tstop->analyzer_done : 0;
}

sig_atomic_t volatile* tstop_get_printer(Thread_stoppers* tstop)
{
    return tstop ? &tstop->printer_done : 0;
}

struct Thread_checkers {
    atomic_bool reader_active;
    atomic_bool analyzer_active;
    atomic_bool printer_active;
};

Thread_checkers* tcheck_create()
{
    Thread_checkers* tcheck = malloc(sizeof(*tcheck));
    if (!tcheck)
        return 0;

    atomic_init(&tcheck->reader_active, false);
    atomic_init(&tcheck->analyzer_active, false);
    atomic_init(&tcheck->printer_active, false);

    return tcheck;
}

void tcheck_destroy(Thread_checkers* tcheck)
{
    free(tcheck);
}

void tcheck_reader_activate(Thread_checkers* tcheck)
{
    if (!tcheck)
        return;
    
    atomic_store(&tcheck->reader_active, true);
}

void tcheck_analyzer_activate(Thread_checkers* tcheck)
{
    if (!tcheck)
        return;
    
    atomic_store(&tcheck->analyzer_active, true);
}

void tcheck_printer_activate(Thread_checkers* tcheck)
{
    if (!tcheck)
        return;
    
    atomic_store(&tcheck->printer_active, true);
}

bool tcheck_check(Thread_checkers* tcheck)
{
    if (!tcheck)
        return false;

    return atomic_load(&tcheck->reader_active) 
        && atomic_load(&tcheck->analyzer_active) 
        && atomic_load(&tcheck->printer_active);
}

void tcheck_reset(Thread_checkers* tcheck)
{
    if (!tcheck)
        return;

    atomic_store(&tcheck->reader_active, false);
    atomic_store(&tcheck->analyzer_active, false);
    atomic_store(&tcheck->printer_active, false);
}