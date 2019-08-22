#pragma once
#include "debug_types.h"

typedef backtrace_t(*func_get_backtrace_t)(uint32_t num_frames);

void debug_init(func_get_backtrace_t get_backtrace);
void debug_error(const char* msg, ...);
void debug_info(const char* msg, ...);
backtrace_t debug_get_backtrace(uint32_t size);

#define error(msg, ...) debug_error(msg, ##__VA_ARGS__)
#define info(msg, ...) debug_info(msg, ##__VA_ARGS__)
#define check(cond, msg, ...) (cond? 0: debug_error(msg, ##__VA_ARGS__))

#ifdef ENABLE_SLOW_DEBUG_CHECKS
#define check_slow(cond, msg, ...) (cond? 0: debug_error(msg, ##__VA_ARGS__))
#else
#define check_slow(cond, msg, ...)
#endif