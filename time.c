#include "time.h"

static float G_time_dt = 0;
static float G_time_since_start = 0;

float time_dt()
{
    return G_time_dt;
}

float time_since_start()
{
    return G_time_since_start;
}

void set_frame_timers(float dt, float since_start)
{
    G_time_dt = dt;
    G_time_since_start = since_start;
}