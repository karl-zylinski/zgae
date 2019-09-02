#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/X.h>
#include <X11/extensions/Xcomposite.h>
#include "window_types.h"
#include "renderer.h"
#include "keyboard_types.h"
#include "debug.h"
#include "memory.h"
#include "time.h"
#include <time.h>
#include "keyboard.h"
#include <execinfo.h>
#include "physics.h"
#include "mouse.h"
#include "game_root.h"

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

    Display* d = XOpenDisplay(NULL);
    i32 s = XDefaultScreen(d);
    
    XSetWindowAttributes attrs = {};
    attrs.background_pixel = BlackPixel(d, s);
    attrs.border_pixel = BlackPixel(d, s);
    attrs.event_mask = ExposureMask
                     | KeyPressMask
                     | KeyReleaseMask
                     | FocusChangeMask
                     | StructureNotifyMask
                     | PointerMotionMask
                     | EnterWindowMask;

    Window w = XCreateWindow(d, XRootWindow(d, s), 
                             0, 0, 1280, 720, 10, CopyFromParent, InputOutput, 
                             CopyFromParent,  CWBackPixel | CWEventMask | CWBorderPixel, &attrs);

    XMapWindow(d, w);

    GenericWindowInfo wi = {
        .display = d,
        .handle = w
    };

    renderer_init(WINDOW_TYPE_X11, wi);
    physics_init();

    info("Starting timers");
    f32 start_time = get_cur_time_seconds();
    f32 last_frame_time = start_time;

    game_init();

    info("Entering main loop");
    XEvent e;
    bool open = true;

    while (open)
    {
        f32 cur_time = get_cur_time_seconds();
        f32 dt = cur_time - last_frame_time;
        f32 since_start = cur_time - start_time;
        last_frame_time = cur_time;
        set_frame_timers(dt, since_start);

        renderer_wait_for_new_frame();

        if (XPending(d) > 0) {
            XNextEvent(d, &e);
        }

        if (window_resize_handled == false && time_since_start() > window_resized_at + 0.5f)
        {
            window_resize_handled = true;
            renderer_surface_resized(window_resize_w, window_resize_h);
        }

        game_update();
    }

    game_shutdown();
    info("Main loop exited, shutting down");
    physics_shutdown();
    renderer_shutdown();
    XDestroyWindow(d, w);
    XCloseDisplay(d);
    memory_check_leaks();
    info("Shutdown finished");
    return 0;
}