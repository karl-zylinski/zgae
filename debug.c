#include "debug.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "debug_types.h"

static func_get_backtrace_t g_get_backtrace = NULL;

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
    check(g_get_backtrace, "Please run debug_init with function that returns backtraces as parameter.");

    backtrace_t bt = g_get_backtrace(2);
    fprintf(stderr, "ERROR IN %s --- ", bt.function_calls[1]);
    free(bt.function_calls);
    
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

void debug_init(func_get_backtrace_t get_backtrace)
{
    g_get_backtrace = get_backtrace;
}

backtrace_t debug_get_backtrace(uint32_t size)
{
    return g_get_backtrace(size);
}