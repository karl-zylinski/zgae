#include "time.h"

static f32 g_time_dt = 0;
static f32 g_time_since_start = 0;

float time_dt()
{
    return g_time_dt;
}

float time_since_start()
{
    return g_time_since_start;
}

void set_frame_timers(f32 dt, f32 since_start)
{
    g_time_dt = dt;
    g_time_since_start = since_start;
}