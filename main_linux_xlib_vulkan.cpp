#include <X11/Xlib.h>
#include "log.h"
#include "memory.h"
#include "time.h"
#include <time.h>
#include "keyboard.h"
#include <execinfo.h>
#include "physics.h"
#include "renderer.h"
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

static Key x11_keycode_to_keycode(u32 code);

int main()
{
    info("Starting ZGAE");
    debug_init(get_backtrace);
    memory_init();
    keyboard_init();

    Display* display = XOpenDisplay(NULL);
    i32 screen = XDefaultScreen(display);
    
    XSetWindowAttributes attrs = {};
    attrs.background_pixel = BlackPixel(display, screen);
    attrs.border_pixel = BlackPixel(display, screen);
    attrs.event_mask = ExposureMask
                     | KeyPressMask
                     | KeyReleaseMask
                     | FocusChangeMask
                     | StructureNotifyMask
                     | PointerMotionMask
                     | EnterWindowMask;

    u32 width = 1280;
    u32 height = 720;
    Window window = XCreateWindow(display, XRootWindow(display, screen), 
                             0, 0, width, height, 10, CopyFromParent, InputOutput, 
                             CopyFromParent,  CWBackPixel | CWEventMask | CWBorderPixel, &attrs);

    // Hide cursor
    XColor xcolor;
    static char csr_bits[] = {0x00};
    Pixmap csr = XCreateBitmapFromData(display,window,csr_bits,1,1);
    Cursor cursor = XCreatePixmapCursor(display,csr,csr,&xcolor,&xcolor,1,1); 
    XDefineCursor(display, window, cursor);

    XStoreName(display, window, "ZGAE");
    XMapWindow(display, window);

    GenericWindowInfo wi = {
        .display = display,
        .handle = window
    };

    renderer_init(WINDOW_TYPE_X11, wi);
    physics_init();

    info("Starting timers");
    f32 start_time = get_cur_time_seconds();
    f32 last_frame_time = start_time;

    game_init();

    info("Entering main loop");

    bool open = true;
    bool closed_by_wm = false;
    bool has_focus = false;

    // Makes ClientMessage trigger when window is clsoed
    Atom wm_delete_atom = XInternAtom(display, "WM_DELETE_WINDOW", True);
    XSetWMProtocols(display, window, &wm_delete_atom, 1);

    XEvent evt;
    while (open)
    {
        f32 cur_time = get_cur_time_seconds();
        f32 dt = cur_time - last_frame_time;
        f32 since_start = cur_time - start_time;
        last_frame_time = cur_time;
        set_frame_timers(dt, since_start);

        while (XPending(display) > 0) {
            XNextEvent(display, &evt);

            switch(evt.type)
            {
                case KeyPress: {
                    let xlib_code = evt.xkey.keycode;
                    let kc = x11_keycode_to_keycode(xlib_code);
                    if (kc == KEY_UNKNOWN)
                        info("Unknown key pressed: %d", xlib_code);
                    keyboard_key_pressed(kc);
                } break;

                case KeyRelease: {
                    // Removes key repeat
                    if (XEventsQueued(display, QueuedAfterReading))
                    {
                        XEvent evt_next;
                        XPeekEvent(display, &evt_next);

                        if (evt_next.type == KeyPress
                            && evt_next.xkey.time == evt.xkey.time
                            && evt_next.xkey.keycode == evt.xkey.keycode)
                        {
                            XNextEvent(display, &evt_next);
                            break;
                        }
                    }

                    let xlib_code = evt.xkey.keycode;
                    let kc = x11_keycode_to_keycode(xlib_code);
                    if (kc == KEY_UNKNOWN)
                        info("Unknown key pressed: %d", xlib_code);
                    keyboard_key_released(kc);
                } break;

                case ClientMessage:
                {
                    closed_by_wm = true;
                    info("Xlib window closed"); // Should this do some check against wm_delete_atom?
                    open = false;
                } break;

                case FocusIn: {
                    has_focus = true;
                } break;

                case FocusOut: {
                    keyboard_reset();
                    has_focus = false;
                } break;

                case MotionNotify: {
                    if (!has_focus)
                        break;

                    let me = evt.xmotion;
                    let x = me.x;
                    let y = me.y;
                    let fake_origin_x = width/2;
                    let fake_origin_y = height/2;
                    let dx = x - fake_origin_x;
                    let dy = y - fake_origin_y;
                    mouse_moved(dx, dy);

                    // Warp pointer to center, but disable events and then re-enable them so stuff doesn't go loopsie.
                    XSetWindowAttributes avoid_warp_message_attrs;
                    avoid_warp_message_attrs.event_mask = 0;
                    XChangeWindowAttributes(display, window, CWEventMask, &avoid_warp_message_attrs);
                    XWarpPointer(display, None, window, 0, 0, 0, 0, fake_origin_x, fake_origin_y);
                    XChangeWindowAttributes(display, window, CWEventMask, &attrs);
                } break;

                case ConfigureNotify: {
                    let conf = evt.xconfigure;

                    if ((u32)conf.width != width || (u32)conf.height != height)
                    {
                        info("X11 window resized to %d x %d", width, height);
                        width = conf.width;
                        height = conf.height;
                        window_resized_at = time_since_start();
                        window_resize_handled = false;
                    }
                } break;
            }
        }

        if (window_resize_handled == false && time_since_start() > window_resized_at + 0.5f)
        {
            window_resize_handled = true;
            renderer_surface_resized(width, height);
        }

        bool cont = game_update();

        if (!cont)
            open = false;
    }

    game_shutdown();
    info("Main loop exited, shutting down");
    physics_shutdown();
    renderer_shutdown();

    if (!closed_by_wm) // May crash because display is already gone if we dont do this
    {
        XDestroyWindow(display, window);
        //XCloseDisplay(display); // This always crashes?????
    }

    memory_check_leaks();
    info("Shutdown finished");
    return 0;
}

static Key x11_keycode_to_keycode(u32 code)
{
    static const Key codes[] = {
        KEY_UNKNOWN, // 0
        KEY_UNKNOWN, // 1
        KEY_UNKNOWN, // 2
        KEY_UNKNOWN, // 3
        KEY_UNKNOWN, // 4
        KEY_UNKNOWN, // 5
        KEY_UNKNOWN, // 6
        KEY_UNKNOWN, // 7
        KEY_UNKNOWN, // 8
        KEY_ESCAPE, // 9
        KEY_UNKNOWN, // 10
        KEY_UNKNOWN, // 11
        KEY_UNKNOWN, // 12
        KEY_UNKNOWN, // 13
        KEY_UNKNOWN, // 14
        KEY_UNKNOWN, // 15
        KEY_UNKNOWN, // 16
        KEY_UNKNOWN, // 17
        KEY_UNKNOWN, // 18
        KEY_UNKNOWN, // 19
        KEY_UNKNOWN, // 20
        KEY_UNKNOWN, // 21
        KEY_UNKNOWN, // 22
        KEY_UNKNOWN, // 23
        KEY_Q, // 24
        KEY_W, // 25
        KEY_E, // 26
        KEY_R, // 27
        KEY_T, // 28
        KEY_Y, // 29
        KEY_U, // 30
        KEY_I, // 31
        KEY_O, // 32
        KEY_P, // 33
        KEY_LBRACKET, // 34
        KEY_RBRACKET, // 35
        KEY_UNKNOWN, // 36
        KEY_UNKNOWN, // 37
        KEY_A, // 38
        KEY_S, // 39
        KEY_D, // 40
        KEY_F, // 41
        KEY_G, // 42
        KEY_H, // 43
        KEY_J, // 44
        KEY_K, // 45
        KEY_L, // 46
        KEY_SEMICOLON, // 47
        KEY_SINGLEQUOTE, // 48
        KEY_UNKNOWN, // 49
        KEY_UNKNOWN, // 50
        KEY_BACKSLASH, // 51
        KEY_Z, // 52
        KEY_X, // 53
        KEY_C, // 54
        KEY_V, // 55
        KEY_B, // 56
        KEY_N, // 57
        KEY_M, // 58
        KEY_COMMA, // 59
        KEY_PERIOD, // 60
        KEY_SLASH, // 61
        KEY_UNKNOWN, // 62
        KEY_UNKNOWN, // 63
        KEY_UNKNOWN, // 64
        KEY_SPACE, // 65
        KEY_UNKNOWN, // 66
        KEY_UNKNOWN, // 67
        KEY_UNKNOWN, // 68
        KEY_UNKNOWN, // 69
        KEY_UNKNOWN, // 70
        KEY_UNKNOWN, // 71
        KEY_UNKNOWN, // 72
        KEY_UNKNOWN, // 73
        KEY_UNKNOWN, // 74
        KEY_UNKNOWN, // 75
        KEY_UNKNOWN, // 76
        KEY_UNKNOWN, // 77
        KEY_UNKNOWN, // 78
        KEY_UNKNOWN, // 79
        KEY_UNKNOWN, // 80
        KEY_UNKNOWN, // 81
        KEY_UNKNOWN, // 82
        KEY_UNKNOWN, // 83
        KEY_UNKNOWN, // 84
        KEY_UNKNOWN, // 85
        KEY_UNKNOWN, // 86
        KEY_UNKNOWN, // 87
        KEY_UNKNOWN, // 88
        KEY_UNKNOWN, // 89
        KEY_UNKNOWN, // 90
        KEY_UNKNOWN, // 91
        KEY_UNKNOWN, // 92
        KEY_UNKNOWN, // 93
        KEY_UNKNOWN, // 94
        KEY_UNKNOWN, // 95
        KEY_UNKNOWN, // 96
        KEY_UNKNOWN, // 97
        KEY_UNKNOWN, // 98
        KEY_UNKNOWN, // 99
        KEY_UNKNOWN, // 100
        KEY_UNKNOWN, // 101
        KEY_UNKNOWN, // 102
        KEY_UNKNOWN, // 103
        KEY_UNKNOWN, // 104
        KEY_UNKNOWN, // 105
        KEY_UNKNOWN, // 106
        KEY_UNKNOWN, // 107
        KEY_UNKNOWN, // 108
        KEY_UNKNOWN, // 109
        KEY_UNKNOWN, // 110
        KEY_UNKNOWN, // 111
        KEY_UNKNOWN, // 112
        KEY_UNKNOWN, // 113
        KEY_UNKNOWN, // 114
        KEY_UNKNOWN, // 115
        KEY_UNKNOWN, // 116
        KEY_UNKNOWN, // 117
        KEY_UNKNOWN, // 118
        KEY_UNKNOWN, // 119
        KEY_UNKNOWN, // 120
        KEY_UNKNOWN, // 121
        KEY_UNKNOWN, // 122
        KEY_UNKNOWN, // 123
        KEY_UNKNOWN, // 124
        KEY_UNKNOWN, // 125
        KEY_UNKNOWN, // 126
        KEY_UNKNOWN, // 127
    };

    u32 num_codes = sizeof(codes)/sizeof(Key);

    if ((u32)code > num_codes)
        return KEY_UNKNOWN;

    return codes[(u32)code];
}