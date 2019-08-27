#pragma once
// This file is (the only) foracbly included by the compiler!

#include <stdint.h>
#include <stdlib.h>


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

typedef i64 hash64;


// Handles

// each handle is of this format (bits):
// iiiiiiii iiiiiiii iiiiiiii iiiiiiii ppppssss ssssssss gggggggg gggggggg
// i = index
// p = pool (ex ResoruceHandle, RenderResourceHandle, etc!)
// s = subtype (ex RenderResourcShader etc)
// g = generation, bumped every time the slot changes in the pool (for checking if the handle is dead etc)

typedef u64 Handle;
static Handle HANDLE_INVALID = -1;
#define handle_index(h) (h >> 32)
#define handle_pool(h) ((h & 0xffffffff) >> 28)
#define handle_type(h) ((h & 0xfffffff) >> 16)
#define handle_generation(h) (h & 0xffff)

#define HANDLE_MAX_INDEX 0xffffffff
#define HANDLE_MAX_POOL_INDEX 0xf
#define HANDLE_MAX_TYPE_INDEX 0xfff
#define HANDLE_MAX_GENERATION 0xffff

#define SMALL_NUMBER 0.00001f


// Forward declaration helpers

#define fwd_struct(t) struct t
#define fwd_enum(e) enum struct e
#define fwd_handle(h) typedef Handle h
#define arrnum(a) sizeof(a)/sizeof(a[0])


// Clang static analyser stuff

#define ANALYZER_NORETURN __attribute__((analyzer_noreturn))


// Misc

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)