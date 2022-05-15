#include "stat_utils.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

char** util_str_split(char* restrict data, const char delimiter, size_t* token_count)
{
    if (!data)
        return 0;

    if (!*token_count) {
        char* symbol = data;

        while (*symbol) {
            if (*symbol == delimiter)
                ++(*token_count);

            ++symbol;
        }
    }

    char** result = malloc(sizeof(*result) * (*token_count));
    if (!result)
        return 0;

    size_t idx = 0;
    char delim[2] = {delimiter, '\0'};
    char* token = strtok(data, delim);
    while (token && idx < *token_count) {
        result[idx] = strdup(token);
        token = strtok(0, delim);
        ++idx;
    }

    *token_count = idx;

    return result;
}

void util_split_cleanup(char** data_tokenized, size_t token_count)
{
    for (size_t i = 0; i < token_count; i++) {
        free(data_tokenized[i]);
    }
    free(data_tokenized);
}

char* util_str_concat(char const* restrict str1, char const* restrict str2)
{
    const size_t s1_len = strlen(str1);
    const size_t s2_len = strlen(str2);

    char* res = malloc(s1_len + s2_len + 1);
    if (!res)
        return 0;
    
    memcpy(res, str1, s1_len);
    memcpy(res + s1_len, str2, s2_len);
    res[s1_len + s2_len] = '\0';
    
    return res;
}

bool reader_starts_with(const char *a, const char *b)
{
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}
