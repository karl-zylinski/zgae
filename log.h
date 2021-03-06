#pragma once

struct Backtrace
{
    char** function_calls;
    u32 function_calls_num;
};

typedef Backtrace(*GetBacktraceCallback)(u32 num_frames);

void debug_init(GetBacktraceCallback get_backtrace);
[[ noreturn ]] void debug_error(const char* msg, ...);
void debug_info(const char* msg, ...);
Backtrace debug_get_backtrace(u32 size);

#define error(msg, ...) debug_error(msg, ##__VA_ARGS__)
#define info(msg, ...) debug_info(msg, ##__VA_ARGS__)
#define check(cond, msg, ...) (cond ? (void)0 : debug_error(msg, ##__VA_ARGS__))

#ifdef ENABLE_SLOW_DEBUG_CHECKS
#define check_slow(cond, msg, ...) (cond ? (void)0 : debug_error(msg, ##__VA_ARGS__))
#else
#define check_slow(cond, msg, ...)
#endif