#ifndef HELPERS_H
# define HELPERS_H

#include "define.h"

int atoi(const char* str);
size_t split_words(char* line, char** argv, size_t max_args);
int ft_strcmp(const char* a, const char* b);
size_t ft_strlcpy(char* dst, const char* src, size_t dst_size);
size_t strlen(const char* str);

#endif