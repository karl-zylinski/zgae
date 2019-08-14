#pragma once
// This file is (the only) foracbly included by the compiler!

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef uint64_t handle_t;
static const handle_t HANDLE_INVALID = -1;
#define fwd_struct(t) typedef struct t t
#define fwd_enum(e) typedef enum e e
#define fwd_handle(h) typedef handle_t h