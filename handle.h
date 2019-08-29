#pragma once

static Handle HANDLE_INVALID = -1;
#define handle_index(h) (h >> 32)
#define handle_pool(h) ((h & 0xffffffff) >> 28)
#define handle_type(h) ((h & 0xfffffff) >> 16)
#define handle_generation(h) (h & 0xffff)

#define HANDLE_MAX_INDEX 0xffffffff
#define HANDLE_MAX_POOL_INDEX 0xf
#define HANDLE_MAX_TYPE_INDEX 0xfff
#define HANDLE_MAX_GENERATION 0xffff
