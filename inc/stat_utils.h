#ifndef STAT_UTILS_H
#define STAT_UTILS_H

#include <stddef.h>
#include <stdbool.h>

char** util_str_split(char* restrict data, const char delimiter, size_t* token_count);
void util_split_cleanup(char** data_tokenized, size_t token_count);
char* util_str_concat(char const* restrict str1, char const* restrict str2);
bool reader_starts_with(const char *a, const char *b);

#endif
