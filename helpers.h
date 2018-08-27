#pragma once
#include <assert.h>
#include <string.h>
#define Assert(cond, msg) assert(cond && msg)
#define Error(msg) assert(false && msg)
#define SmallNumber 0.0000001f
#define InvalidHandle (unsigned int)(-1)
#define memzero(ptr, size) memset(ptr, 0, size)