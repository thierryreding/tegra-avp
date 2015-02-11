#ifndef STRING_H
#define STRING_H

#include <types.h>

unsigned long strtoul(const char *nptr, const char **endptr, int base);

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

#endif
