#pragma once

struct Backtrace
{
    char** function_calls;
    u32 function_calls_num;
};

typedef Backtrace(*GetBacktraceCallback)(u32 num_frames);