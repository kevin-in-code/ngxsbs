
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "error.h"

void error(const char* str, ...)
{
	va_list argp;
	va_start(argp, str);
	vfprintf(stderr, str, argp);
	va_end(argp);
	exit(EXIT_FAILURE);
}
