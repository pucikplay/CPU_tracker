#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define STAT_PATH "/proc/stat"

bool reader_starts_with(const char *a, const char *b) {
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}

size_t reader_readstat(char** buff, size_t buff_len, FILE* stat_file) {

    if (!stat_file || !buff) {
        printf("Reader: invalid buffer or file pointer.\n");
        return 0;
    }

    
}