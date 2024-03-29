#ifndef COMMON_H
#define COMMON_H
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include "types.h"

bool str_ends_with(const char *haystack, const char *needle)
{
    size_t needle_length = strlen(needle);
    size_t haystack_length = strlen(haystack);
    char *p = strstr(haystack, needle);

    return (p && (haystack + haystack_length) - p == (ptrdiff_t)needle_length);
}

int cmpfilename(const void *a, const void *b)
{
    return strcmp(((fileinfo_t*)a)->filename, ((fileinfo_t*)b)->filename);
}

#endif
