#ifndef STDIO_H
#define STDIO_H

#include <stdarg.h>
#include <types.h>

int vsnprintf(char *str, size_t size, const char *format, va_list ap);

#endif
