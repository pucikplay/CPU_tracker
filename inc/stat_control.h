#ifndef STAT_CONTROL_H
#define STAT_CONTROL_H

#include <signal.h>
#include <stdatomic.h>

typedef struct Thread_stoppers Thread_stoppers;

Thread_stoppers* tstop_create();
void tstop_destroy(Thread_stoppers* tstop);
void tstop_stop_threads(Thread_stoppers* tstop);
sig_atomic_t volatile* tstop_get_reader(Thread_stoppers* tstop);
sig_atomic_t volatile* tstop_get_analyzer(Thread_stoppers* tstop);
sig_atomic_t volatile* tstop_get_printer(Thread_stoppers* tstop);

typedef struct Thread_checkers Thread_checkers;

Thread_checkers* tcheck_create();
void tcheck_destroy(Thread_checkers* tcheck);
void tcheck_reader_activate(Thread_checkers* tcheck);
void tcheck_analyzer_activate(Thread_checkers* tcheck);
void tcheck_printer_activate(Thread_checkers* tcheck);
bool tcheck_check(Thread_checkers* tcheck);
void tcheck_reset(Thread_checkers* tcheck);

#endif //STAT_CONTROL_H