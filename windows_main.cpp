#include "config.h"
#include "memory.h"
#include "windows_window.h"
#include "renderer_direct3d.h"
#include "game_main.h"
#include "time.h"
#include <Windows.h>

int main()
{
    memory_init();

    WindowsWindow win = {};
    windows_create_window(&win, G_program_name, G_default_window_width, G_default_window_height);
    
    RendererD3D renderer = {};
    renderer.init(win.handle);

    LARGE_INTEGER counter_li;
    QueryPerformanceCounter(&counter_li);
    long long timer_start = counter_li.QuadPart;
    long long timer_last_frame = timer_start;

    LARGE_INTEGER freq_li;
    QueryPerformanceFrequency(&freq_li);
    double timer_freq = ((double)freq_li.QuadPart)/1000.0;
    
    game_start(&win.state, &renderer);

    while(win.state.closed == false)
    {
        LARGE_INTEGER frame_li;
        QueryPerformanceCounter(&frame_li);
        long long cur_counter = frame_li.QuadPart;
        float time_since_start = (float)(double(cur_counter - timer_start) / timer_freq / 1000.0);
        float frame_dt = (float)(double(cur_counter - timer_last_frame) / timer_freq / 1000.0);
        set_frame_timers(frame_dt, time_since_start);
        timer_last_frame = cur_counter;
        windows_process_all_window_messsages();
        game_do_frame();
    }

    game_shutdown();
    renderer.shutdown();
    memory_shutdown();
}