#ifndef INCLUDE_STDIO_H_
#define INCLUDE_STDIO_H_

#include<stdarg.h>
#include <stddef.h>

/* kernel printf */
int printk(const char *fmt, ...);
int vprintk(const char *fmt, va_list va);
int prints(const char *fmt, ...);
int vprints(const char *fmt, va_list va);

/* user printf */
int printf(const char *fmt, ...);
int vprintf(const char *fmt, va_list va);

int puts(const char *str);
int putchar(int ch);

#endif
