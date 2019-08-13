#pragma once

void debug_error(const char* msg, ...);
void debug_info(const char* msg, ...);

#define error(msg, ...) debug_error(msg, ##__VA_ARGS__)
#define info(msg, ...) debug_info(msg, ##__VA_ARGS__)
#define check(cond, msg, ...) (cond? 0: debug_error(msg, ##__VA_ARGS__))

#ifdef ENABLE_SLOW_DEBUG_CHECKS
#define check_slow(cond, msg, ...) (cond? 0: debug_error(msg, ##__VA_ARGS__))
#else
#define check_slow(cond, msg, ...)
#endif
