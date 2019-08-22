#pragma once

typedef struct Backtrace
{
    const char** function_calls;
    u32 function_calls_num;
} Backtrace;

typedef Backtrace(*GetBacktraceCallback)(u32 num_frames);