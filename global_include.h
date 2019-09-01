#pragma once
// This file is (the only) foracbly included by the compiler!

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

typedef i64 hash64;
typedef i8 byte;


// Handles

// each handle is of this format (bits):
// iiiiiiii iiiiiiii iiiiiiii iiiiiiii ppppssss ssssssss gggggggg gggggggg
// i = index
// p = pool (ex ResoruceHandle, RenderResourceHandle, etc!)
// s = subtype (ex RenderResourcShader etc)
// g = generation, bumped every time the slot changes in the pool (for checking if the handle is dead etc)

typedef u64 Handle;


// Forward declaration helpers

#define fwd_struct(t) struct t
#define fwd_enum(e) enum e : u32
#define fwd_handle(h) typedef Handle h


// Misc

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#ifndef NULL
    #define NULL 0
#endif

#define let auto
