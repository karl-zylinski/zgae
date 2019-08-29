#include "debug.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

static GetBacktraceCallback g_get_backtrace = NULL;

static void print_current_time()
{
    time_t rawtime; time (&rawtime);
    struct tm* lt = localtime(&rawtime);
    int hour = lt->tm_hour;
    int minute = lt->tm_min;
    int second = lt->tm_sec;

    fprintf(stderr, "[%02d:%02d:%02d] ", hour, minute, second);
}

void debug_error(char* msg, ...)
{
    print_current_time();
    check(g_get_backtrace, "Please run debug_init with function that returns backtraces as parameter.");

    Backtrace bt = g_get_backtrace(3);
    fprintf(stderr, "ERROR IN %s --- ", bt.function_calls[2]);
    free(bt.function_calls);
    
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    va_end(args);
    abort();
}

void debug_info(char* msg, ...)
{
    print_current_time();
    va_list args;
    va_start(args, msg);
    vfprintf(stderr, msg, args);
    fprintf(stderr, "\n");
    va_end(args);
}

void debug_init(GetBacktraceCallback get_backtrace)
{
    g_get_backtrace = get_backtrace;
}

Backtrace debug_get_backtrace(u32 size)
{
    return g_get_backtrace(size);
}