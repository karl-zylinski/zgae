#include "linux_xcb_window.h"
#include "window_types.h"
#include "renderer.h"
#include "keycode_types.h"
#include "debug.h"
#include "memory.h"
#include "shader_resource_types.h"
#include "math.h"
#include "time.h"
#include <time.h>
#include <math.h>
#include "keyboard.h"
#include <execinfo.h>
#include "obj_loader.h"
#include "resource_store.h"
#include "resource_types.h"

#define XYZ1(_x_, _y_, _z_) {(_x_), (_y_), (_z_), 1.f}

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
    sizet bt_size = backtrace(backtraces, backtrace_size);
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
    RendererState* rs = renderer_create(WINDOW_TYPE_XCB, win);
    resource_store_init(rs);

    WindowCallbacks wc = {};
    wc.key_pressed_callback = &keyboard_key_pressed;
    wc.key_released_callback = &keyboard_key_released;
    wc.focus_lost_callback = &keyboard_reset;
    wc.window_resized_callback = &handle_window_resize;
    linux_xcb_window_update_callbacks(win, &wc);

    ResourceHandle prh = resource_load("pipeline_default.pipeline");
    RendererResourceHandle ph = renderer_load_pipeline(rs, prh);
    ObjLoadResult olr = obj_load("box.wobj");
    check(olr.ok, "Failed loading obj");
    RendererResourceHandle gh = renderer_load_geometry(rs, &olr.mesh);
    memf(olr.mesh.vertices);
    memf(olr.mesh.indices);

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
            f32 fps = ((f32)frames)/d;
            (void)fps;
            //info("%f fps", fps);
            framerate_timer = time_since_start();
            frames = 0;
        }

        if (key_is_held(KC_A))
            camera_pos.x -= time_dt();
        if (key_is_held(KC_D))
            camera_pos.x += time_dt();
        if (key_is_held(KC_W))
            camera_pos.y += time_dt();
        if (key_is_held(KC_S))
            camera_pos.y -= time_dt();
        if (key_is_held(KC_R))
            camera_pos.z += time_dt();
        if (key_is_held(KC_F))
            camera_pos.z -= time_dt();

        renderer_wait_for_new_frame(rs);
        linux_xcb_window_process_all_events(win);

        if (window_resize_handled == false && time_since_start() > window_resized_at + 0.5f)
        {
            window_resize_handled = true;
            renderer_surface_resized(rs, window_resize_w, window_resize_h);
        }

        renderer_draw(rs, ph, gh, &camera_pos, &camera_rot);
        renderer_present(rs);
        keyboard_end_of_frame();
    }

    info("Main loop exited, shutting down");
    renderer_destroy(rs);
    linux_xcb_window_destroy(win);
    resource_store_destroy();
    memory_check_leaks();
    info("Shutdown finished");
    return 0;
}