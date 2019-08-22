
static float s_time_dt = 0;
static float s_time_since_start = 0;

float time_dt()
{
    return s_time_dt;
}

float time_since_start()
{
    return s_time_since_start;
}

void set_frame_timers(float dt, float since_start)
{
    s_time_dt = dt;
    s_time_since_start = since_start;
}