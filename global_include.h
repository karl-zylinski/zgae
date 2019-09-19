#pragma once
// This file is foracbly included by the compiler!

#include <stdint.h>


// Basic types

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef float f32;
typedef double f64;


// Forward declaration helpers

#define fwd_struct(t) struct t
#define fwd_enum(e) enum e : u32


// Misc

#ifndef NULL
    #define NULL 0
#endif

#define let auto

#define arr_foreach(var, arr, num) for(let var = arr; var < (arr + num); ++var)
#define arr_idx(var, arr) (((u32)(((i8*)var)-((i8*)arr)))/(sizeof(*var)))


// Defer

template <typename F>
struct ScopeGuard {
    ScopeGuard(F f) : f(f) {}
    ~ScopeGuard() { f(); }
    F f;
};

template <typename F>
ScopeGuard<F> make_scope_guard(F f) {
    return ScopeGuard<F>(f);
};

#define CONCAT_(arg1, arg2) arg1 ## arg2
#define CONCAT(arg1, arg2) CONCAT_(arg1, arg2)
#define defer(code) let CONCAT(scope_exit_, __LINE__) = make_scope_guard([=](){code;})
