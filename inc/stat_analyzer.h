#ifndef STAT_ANALYZER_H
#define STAT_ANALYZER_H

#include <stddef.h>

char* analyzer_calc(char* prev_data, char* curr_data);
void* thread_analyze(void *arg);

#endif