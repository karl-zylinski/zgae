#include "linux_xcb_window.h"
#include "window_types.h"
#include "renderer.h"
#include "keycode_types.h"
#include "debug.h"
#include "memory.h"
#include "math.h"
#include "time.h"
#include <time.h>
#include "keyboard.h"
#include <execinfo.h>
#include <math.h>

static f32 get_cur_time_seconds()
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    u32 secs = t.tv_sec;
    return (f32)secs + ((f32)t.tv_nsec)/1000000000.0f;
}

static bool window_resize_handled = true;
static f32 window_resized_at = 0;
static u32 window_resize_w = 0;
static u32 window_resize_h = 0;

static void handle_window_resize(u32 w, u32 h)
{
    window_resize_handled = false;
    window_resized_at = time_since_start();
    window_resize_w = w;
    window_resize_h = h;
}

static Backtrace get_backtrace(u32 backtrace_size)
{
    if (backtrace_size > 32)
        backtrace_size = 32;

    static void* backtraces[32];
    u32 bt_size = backtrace(backtraces, backtrace_size);
    const char** bt_symbols = (const char**)backtrace_symbols(backtraces, bt_size);
    Backtrace bt = {
        .function_calls = bt_symbols,
        .function_calls_num = bt_size
    };
    return bt;
}

int main()
{
    info("Starting ZGAE");
    debug_init(get_backtrace);
    memory_init();
    keyboard_init();
    XcbWindow* win = linux_xcb_window_create("ZGAE", 640, 480);
    Renderer* rs = renderer_create(WindowType::Xcb, win);
    RenderResourceHandle rw = renderer_create_world(rs);

    WindowCallbacks wc = {};
    wc.key_pressed_callback = &keyboard_key_pressed;
    wc.key_released_callback = &keyboard_key_released;
    wc.focus_lost_callback = &keyboard_reset;
    wc.window_resized_callback = &handle_window_resize;
    linux_xcb_window_update_callbacks(win, &wc);

    RenderResourceHandle ph = renderer_resource_load(rs, "pipeline_default.pipeline");
    RenderResourceHandle gh = renderer_resource_load(rs, "box.mesh");

    Mat4 model = mat4_identity();
    u32 box_world_idx = renderer_world_add(rs, rw, gh, &model);

    info("Starting timers");
    
    f32 start_time = get_cur_time_seconds();
    f32 last_frame_time = start_time;
    f32 framerate_timer = 0;
    u32 frames = 0;

    info("Entering main loop");

    Quat camera_rot = quat_identity();
    Vec3 camera_pos = {0, -3, 0};

    while (linux_xcb_window_is_open(win))
    {
        f32 cur_time = get_cur_time_seconds();
        f32 dt = cur_time - last_frame_time;
        f32 since_start = cur_time - start_time;
        last_frame_time = cur_time;
        set_frame_timers(dt, since_start);
        frames = frames + 1;

        if (time_since_start() > (framerate_timer + 2))
        {
            f32 d = time_since_start() - framerate_timer;
            f32 ft = d/((f32)frames);
            (void)ft;
            //info("%f ms", ft*1000.0f);
            framerate_timer = time_since_start();
            frames = 0;
        }

        if (key_is_held(KeyCode::A))
            camera_pos.x -= time_dt();
        if (key_is_held(KeyCode::D))
            camera_pos.x += time_dt();
        if (key_is_held(KeyCode::W))
            camera_pos.y += time_dt();
        if (key_is_held(KeyCode::S))
            camera_pos.y -= time_dt();
        if (key_is_held(KeyCode::R))
            camera_pos.z += time_dt();
        if (key_is_held(KeyCode::F))
            camera_pos.z -= time_dt();

        model.w.x = sin(time_since_start());
        model.w.z = cos(time_since_start());

        renderer_world_move(rs, rw, box_world_idx, &model);

        renderer_wait_for_new_frame(rs);
        linux_xcb_window_process_all_events(win);

        if (window_resize_handled == false && time_since_start() > window_resized_at + 0.5f)
        {
            window_resize_handled = true;
            renderer_surface_resized(rs, window_resize_w, window_resize_h);
        }

        renderer_draw_world(rs, ph, rw, camera_pos, camera_rot);
        renderer_present(rs);
        keyboard_end_of_frame();
    }

    info("Main loop exited, shutting down");
    renderer_destroy(rs);
    linux_xcb_window_destroy(win);
    memory_check_leaks();
    info("Shutdown finished");
    return 0;
}