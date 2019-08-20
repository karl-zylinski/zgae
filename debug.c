#include "debug.h"
#include <stdio.h>
#include <stdarg.h>
#include <execinfo.h>
#include <time.h>

static void print_current_time()
{
    time_t rawtime; time (&rawtime);
    struct tm* lt = localtime(&rawtime);
    int hour = lt->tm_hour;
    int minute = lt->tm_min;
    int second = lt->tm_sec;

    fprintf(stderr, "[%02d:%02d:%02d] ", hour, minute, second);
}

void debug_error(const char* msg, ...)
{
    print_current_time();

    void* backtraces[2];
    size_t bt_size = backtrace(backtraces, 2);
    char** bt_symbols = backtrace_symbols(backtraces, bt_size);
    fprintf(stderr, "ERROR IN %s --- ", bt_symbols[1]);
    free(bt_symbols);

    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}

void debug_info(const char* msg, ...)
{
    print_current_time();
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    va_end(args);
}