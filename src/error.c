//
// Created by Kilig on 2024/11/21.
//


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

#include "error.h"

void warning(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
    vfprintf(stderr, fmt, args);
	va_end(args);
}

noreturn void error(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	exit(EXIT_FAILURE);
}

noreturn void panic(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	abort();
}
