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
#include "obj_loader.h"
#include "gjk_epa.h"

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
    char** bt_symbols = backtrace_symbols(backtraces, bt_size);

    return {
        .function_calls = bt_symbols,
        .function_calls_num = bt_size
    };
}

int main()
{
    info("Starting ZGAE");
    debug_init(get_backtrace);
    memory_init();
    keyboard_init();
    XcbWindow* win = linux_xcb_window_create("ZGAE", 640, 480);
    Renderer* rs = renderer_create(WINDOW_TYPE_XCB, win);
    RenderResourceHandle rw = renderer_create_world(rs);

    WindowCallbacks wc = {};
    wc.key_pressed_callback = &keyboard_key_pressed;
    wc.key_released_callback = &keyboard_key_released;
    wc.focus_lost_callback = &keyboard_reset;
    wc.window_resized_callback = &handle_window_resize;
    linux_xcb_window_update_callbacks(win, wc);

    RenderResourceHandle ph = renderer_resource_load(rs, "pipeline_default.pipeline");
    RenderResourceHandle gh = renderer_resource_load(rs, "box.mesh");

    ObjLoadVerticesResult olr = obj_load_vertices("box.wobj");

    check(olr.ok, "failed loading physics mesh");

    Vec3 p1 = {0, 0, 0};
    Vec3 p2 = {-2, 0, 0};

    GjkShape gs1 = {
        .vertices = olr.vertices,
        .vertices_num = olr.vertices_num,
        .position = p1
    };

    GjkShape gs2 = {
        .vertices = olr.vertices,
        .vertices_num = olr.vertices_num,
        .position = p2
    };

    Mat4 m1 = mat4_identity();
    Mat4 m2 = mat4_identity();

    m2.w.x = p2.x;

    u32 b1_world_idx = renderer_world_add(rs, rw, gh, m1);
    (void)b1_world_idx;
    u32 b2_world_idx = renderer_world_add(rs, rw, gh, m2);

    info("Starting timers");
    
    f32 start_time = get_cur_time_seconds();
    f32 last_frame_time = start_time;
    f32 framerate_timer = 0;
    u32 frames = 0;

    info("Entering main loop");

    Quat camera_rot = quat_identity();
    Vec3 camera_pos = {0, -8, 0};

    while (linux_xcb_window_is_open(win) && !key_held(KC_ESCAPE))
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

        if (key_held(KC_A))
            p2.x -= time_dt();
        if (key_held(KC_D))
            p2.x += time_dt();
        if (key_held(KC_W))
            p2.y += time_dt();
        if (key_held(KC_S))
            p2.y -= time_dt();
        if (key_held(KC_R))
            p2.z += time_dt();
        if (key_held(KC_F))
            p2.z -= time_dt();

        gs2.position = p2;

        m2.w.x = p2.x;
        m2.w.y = p2.y;
        m2.w.z = p2.z;
        renderer_world_move(rs, rw, b2_world_idx, m2);

        bool collision = gjk_intersect(gs1, gs2);

        if (collision)
        {
            info("COLLISION");
        }

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

    memf(olr.vertices);
    info("Main loop exited, shutting down");
    renderer_destroy(rs);
    linux_xcb_window_destroy(win);
    memory_check_leaks();
    info("Shutdown finished");
    return 0;
}