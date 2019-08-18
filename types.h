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

typedef uint32_t handle_t;
static const handle_t HANDLE_INVALID = -1;
#define handle_index(h) (h >> 8)
#define handle_type(h) ((h & 0xff) >> 1)
#define handle_used(h) (h & 0x1)

#define HANDLE_MAX_INDEX 16777216
#define HANDLE_MAX_TYPE_INDEX 128

#define fwd_struct(t) typedef struct t t
#define fwd_enum(e) typedef enum e e
#define fwd_handle(h) typedef handle_t h
