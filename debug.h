#pragma once
#include "debug_types.h"

void debug_init(GetBacktraceCallback get_backtrace);
[[ noreturn ]] void debug_error(char* msg, ...);
void debug_info(char* msg, ...);
Backtrace debug_get_backtrace(u32 size);

#define error(msg, ...) debug_error(msg, ##__VA_ARGS__)
#define info(msg, ...) debug_info(msg, ##__VA_ARGS__)
#define check(cond, msg, ...) (cond ? (void)0 : debug_error(msg, ##__VA_ARGS__))

#ifdef ENABLE_SLOW_DEBUG_CHECKS
#define check_slow(cond, msg, ...) (cond ? (void)0 : debug_error(msg, ##__VA_ARGS__))
#else
#define check_slow(cond, msg, ...)
#endif