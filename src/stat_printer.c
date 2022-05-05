#include "stat_printer.h"
#include "buffer_sync.h"

#include <stdio.h>
#include <stdlib.h>

void printer_print(char* raw_data)
{
    printf("%s\n", raw_data);
}

void* thread_print(void *arg)
{
    Buff_sync* bs = *(Buff_sync**)arg;

    char* cpu_data = 0;
    bool done = false;

    while (!done) {
        //receive from anlyzer
        buff_sync_lock(bs);
        
        if (buff_sync_is_empty(bs))
            buff_sync_wait_for_producer(bs);

        cpu_data = buff_sync_pop(bs);

        buff_sync_call_producer(bs);
        buff_sync_unlock(bs);

        if (!cpu_data)
            continue;
        
        //print the data
        printer_print(cpu_data);

        //clean
        free(cpu_data);
        cpu_data = 0;
    }


    return NULL;
}