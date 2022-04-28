#include <stdio.h>
#include <stddef.h>

#include "stat_reader.h"

int main(void) {

    char* buff = 0;
    size_t buff_size = 0;
    FILE* stat_file = fopen("/proc/stat", "r");

    size_t cpu_count = reader_read_stat(&buff, &buff_size, stat_file);

    printf("there are %ld cores\n", cpu_count);
    fclose(stat_file);
    printf("%s\n", buff);
}