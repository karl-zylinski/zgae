#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/X.h>
#include <X11/extensions/Xcomposite.h>
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

static Keycode x11_keycode_to_keycode(u32 code);

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

        renderer_wait_for_new_frame();

        while (XPending(display) > 0) {
            XNextEvent(display, &evt);

            switch(evt.type)
            {
                case KeyPress: {
                    info("Key pressed");
                    let xlib_code = evt.xkey.keycode;
                    let kc = x11_keycode_to_keycode(xlib_code);
                    if (kc == KC_UNKNOWN)
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

                    info("Key released");
                    let xlib_code = evt.xkey.keycode;
                    let kc = x11_keycode_to_keycode(xlib_code);
                    if (kc == KC_UNKNOWN)
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

        game_update();
    }

    game_shutdown();
    info("Main loop exited, shutting down");
    physics_shutdown();
    renderer_shutdown();

    if (!closed_by_wm) // May crash because display is already gone if we dont do this
    {
        XDestroyWindow(display, window);
        XCloseDisplay(display);
    }

    memory_check_leaks();
    info("Shutdown finished");
    return 0;
}

static Keycode x11_keycode_to_keycode(u32 code)
{
    static Keycode codes[] = {
        KC_UNKNOWN, // 0
        KC_UNKNOWN, // 1
        KC_UNKNOWN, // 2
        KC_UNKNOWN, // 3
        KC_UNKNOWN, // 4
        KC_UNKNOWN, // 5
        KC_UNKNOWN, // 6
        KC_UNKNOWN, // 7
        KC_UNKNOWN, // 8
        KC_ESCAPE, // 9
        KC_UNKNOWN, // 10
        KC_UNKNOWN, // 11
        KC_UNKNOWN, // 12
        KC_UNKNOWN, // 13
        KC_UNKNOWN, // 14
        KC_UNKNOWN, // 15
        KC_UNKNOWN, // 16
        KC_UNKNOWN, // 17
        KC_UNKNOWN, // 18
        KC_UNKNOWN, // 19
        KC_UNKNOWN, // 20
        KC_UNKNOWN, // 21
        KC_UNKNOWN, // 22
        KC_UNKNOWN, // 23
        KC_Q, // 24
        KC_W, // 25
        KC_E, // 26
        KC_R, // 27
        KC_T, // 28
        KC_Y, // 29
        KC_U, // 30
        KC_I, // 31
        KC_O, // 32
        KC_P, // 33
        KC_LBRACKET, // 34
        KC_RBRACKET, // 35
        KC_UNKNOWN, // 36
        KC_UNKNOWN, // 37
        KC_A, // 38
        KC_S, // 39
        KC_D, // 40
        KC_F, // 41
        KC_G, // 42
        KC_H, // 43
        KC_J, // 44
        KC_K, // 45
        KC_L, // 46
        KC_SEMICOLON, // 47
        KC_SINGLEQUOTE, // 48
        KC_UNKNOWN, // 49
        KC_UNKNOWN, // 50
        KC_BACKSLASH, // 51
        KC_Z, // 52
        KC_X, // 53
        KC_C, // 54
        KC_V, // 55
        KC_B, // 56
        KC_N, // 57
        KC_M, // 58
        KC_COMMA, // 59
        KC_PERIOD, // 60
        KC_SLASH, // 61
        KC_UNKNOWN, // 62
        KC_UNKNOWN, // 63
        KC_UNKNOWN, // 64
        KC_UNKNOWN, // 65
        KC_UNKNOWN, // 66
        KC_UNKNOWN, // 67
        KC_UNKNOWN, // 68
        KC_UNKNOWN, // 69
        KC_UNKNOWN, // 70
        KC_UNKNOWN, // 71
        KC_UNKNOWN, // 72
        KC_UNKNOWN, // 73
        KC_UNKNOWN, // 74
        KC_UNKNOWN, // 75
        KC_UNKNOWN, // 76
        KC_UNKNOWN, // 77
        KC_UNKNOWN, // 78
        KC_UNKNOWN, // 79
        KC_UNKNOWN, // 80
        KC_UNKNOWN, // 81
        KC_UNKNOWN, // 82
        KC_UNKNOWN, // 83
        KC_UNKNOWN, // 84
        KC_UNKNOWN, // 85
        KC_UNKNOWN, // 86
        KC_UNKNOWN, // 87
        KC_UNKNOWN, // 88
        KC_UNKNOWN, // 89
        KC_UNKNOWN, // 90
        KC_UNKNOWN, // 91
        KC_UNKNOWN, // 92
        KC_UNKNOWN, // 93
        KC_UNKNOWN, // 94
        KC_UNKNOWN, // 95
        KC_UNKNOWN, // 96
        KC_UNKNOWN, // 97
        KC_UNKNOWN, // 98
        KC_UNKNOWN, // 99
        KC_UNKNOWN, // 100
        KC_UNKNOWN, // 101
        KC_UNKNOWN, // 102
        KC_UNKNOWN, // 103
        KC_UNKNOWN, // 104
        KC_UNKNOWN, // 105
        KC_UNKNOWN, // 106
        KC_UNKNOWN, // 107
        KC_UNKNOWN, // 108
        KC_UNKNOWN, // 109
        KC_UNKNOWN, // 110
        KC_UNKNOWN, // 111
        KC_UNKNOWN, // 112
        KC_UNKNOWN, // 113
        KC_UNKNOWN, // 114
        KC_UNKNOWN, // 115
        KC_UNKNOWN, // 116
        KC_UNKNOWN, // 117
        KC_UNKNOWN, // 118
        KC_UNKNOWN, // 119
        KC_UNKNOWN, // 120
        KC_UNKNOWN, // 121
        KC_UNKNOWN, // 122
        KC_UNKNOWN, // 123
        KC_UNKNOWN, // 124
        KC_UNKNOWN, // 125
        KC_UNKNOWN, // 126
        KC_UNKNOWN, // 127
    };

    u32 num_codes = sizeof(codes)/sizeof(Keycode);

    if ((u32)code > num_codes)
        return KC_UNKNOWN;

    return codes[(u32)code];
}