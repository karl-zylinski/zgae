#pragma once

typedef struct backtrace_t
{
    const char** function_calls;
    uint32_t function_calls_num;
} backtrace_t;