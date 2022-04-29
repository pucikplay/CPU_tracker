#include <stdio.h>
#include <stddef.h>
#include <unistd.h>

#include "stat_reader.h"
#include "stat_analyzer.h"

int main(void) {

    char* buff1 = 0;
    char* buff2 = 0;
    size_t buff_size = 0;
    FILE* stat_file;
    size_t cpu_count = 0;
    char* data = 0;

    stat_file = fopen("/proc/stat", "r");
    cpu_count = reader_read_stat(&buff1, &buff_size, stat_file);
    printf("there are %ld cores\n", cpu_count - 1);
    fclose(stat_file);

    printf("%s\n", buff1);

    while(cpu_count < 100000000){cpu_count++;}

    stat_file = fopen("/proc/stat", "r");
    cpu_count = reader_read_stat(&buff2, &buff_size, stat_file);
    printf("there are %ld cores\n", cpu_count - 1);
    fclose(stat_file);

    printf("%s\n", buff2);

    data = analyzer_calc(buff1, buff2);

    printf("%s\n", data);
}