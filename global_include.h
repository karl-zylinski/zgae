#pragma once
// This file is (the only) foracbly included by the compiler!

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// each handle is of this format (bits):
// iiiiiiii iiiiiiii iiiiiiii tttttttu
// i = index
// t = type index (se handle_pool.h, max 128 types!!)
// u = used

#define ANALYZER_NORETURN __attribute__((analyzer_noreturn))

typedef uint32_t Handle;
static const Handle HANDLE_INVALID = -1;
#define handle_index(h) (h >> 8)
#define handle_type(h) ((h & 0xff) >> 1)
#define handle_used(h) (h & 0x1)
#define SMALL_NUMBER 0.00001f

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

typedef size_t sizet;

typedef i64 hash64;

#define HANDLE_MAX_INDEX 16777216
#define HANDLE_MAX_TYPE_INDEX 128

#define fwd_struct(t) typedef struct t t
#define fwd_enum(e) typedef enum e e
#define fwd_handle(h) typedef Handle h