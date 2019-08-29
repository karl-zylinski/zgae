#include "linux_xcb_window.h"
#include <xcb/xcb.h>
#include <string.h>
#include "debug.h"
#include "keycode_types.h"
#include "window_types.h"
#include "memory.h"
#include <stdlib.h>

struct XcbEventQueue
{
    xcb_generic_event_t* prev;
    xcb_generic_event_t* current;
    xcb_generic_event_t* next;
};

struct XcbWindow
{
    xcb_connection_t* connection;
    XcbEventQueue evt_queue;
    u32 handle;
    WindowState state;
};

XcbWindow* linux_xcb_window_create(char* title, u32 width, u32 height)
{
    XcbWindow* w = mema_zero_t(XcbWindow);
    info("Creating XCB window w/ title %s, width %d, height %d", title, width, height);
    w->state.width = width;
    w->state.height = height;
    w->connection = xcb_connect(NULL, NULL);
    xcb_screen_t* screen = xcb_setup_roots_iterator(xcb_get_setup(w->connection)).data;
    w->handle = xcb_generate_id(w->connection);
    u32 mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    u32 values[] = { screen->black_pixel, 
        XCB_EVENT_MASK_EXPOSURE
        | XCB_EVENT_MASK_KEY_PRESS
        | XCB_EVENT_MASK_KEY_RELEASE
        | XCB_EVENT_MASK_FOCUS_CHANGE
        | XCB_EVENT_MASK_STRUCTURE_NOTIFY };
    xcb_create_window(
        w->connection,
        XCB_COPY_FROM_PARENT,
        w->handle,
        screen->root,
        0, 0, width, height,
        10,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        mask, values);
    xcb_map_window(w->connection, w->handle);

    // Set title.
    xcb_change_property(
        w->connection,
        XCB_PROP_MODE_REPLACE,
        w->handle,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,
        strlen(title),
        title);

    // For being able to get window closed event.
    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(w->connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(w->connection, cookie, 0);
    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(w->connection, 0, 16, "WM_DELETE_WINDOW");
    xcb_intern_atom_reply_t* reply2 = xcb_intern_atom_reply(w->connection, cookie2, 0);
    xcb_change_property(w->connection, XCB_PROP_MODE_REPLACE, w->handle, (*reply).atom, 4, 32, 1, &(*reply2).atom);

    xcb_flush(w->connection);
    return w;
}

void linux_xcb_window_destroy(XcbWindow* w)
{
    info("Destroying XCB window");
    xcb_destroy_window(w->connection, w->handle);
    xcb_disconnect(w->connection);
    memf(w);
}

void linux_xcb_window_update_callbacks(XcbWindow* w, WindowCallbacks* wc)
{
    w->state.callbacks = *wc;
}

static KeyCode xcb_key_to_keycode(xcb_keycode_t code)
{
    static KeyCode codes[] = {
        KeyCode::Unknown, // 0
        KeyCode::Unknown, // 1
        KeyCode::Unknown, // 2
        KeyCode::Unknown, // 3
        KeyCode::Unknown, // 4
        KeyCode::Unknown, // 5
        KeyCode::Unknown, // 6
        KeyCode::Unknown, // 7
        KeyCode::Unknown, // 8
        KeyCode::Unknown, // 9
        KeyCode::Unknown, // 10
        KeyCode::Unknown, // 11
        KeyCode::Unknown, // 12
        KeyCode::Unknown, // 13
        KeyCode::Unknown, // 14
        KeyCode::Unknown, // 15
        KeyCode::Unknown, // 16
        KeyCode::Unknown, // 17
        KeyCode::Unknown, // 18
        KeyCode::Unknown, // 19
        KeyCode::Unknown, // 20
        KeyCode::Unknown, // 21
        KeyCode::Unknown, // 22
        KeyCode::Unknown, // 23
        KeyCode::Q, // 24
        KeyCode::W, // 25
        KeyCode::E, // 26
        KeyCode::R, // 27
        KeyCode::T, // 28
        KeyCode::Y, // 29
        KeyCode::U, // 30
        KeyCode::I, // 31
        KeyCode::O, // 32
        KeyCode::P, // 33
        KeyCode::LBracket, // 34
        KeyCode::RBracket, // 35
        KeyCode::Unknown, // 36
        KeyCode::Unknown, // 37
        KeyCode::A, // 38
        KeyCode::S, // 39
        KeyCode::D, // 40
        KeyCode::F, // 41
        KeyCode::G, // 42
        KeyCode::H, // 43
        KeyCode::J, // 44
        KeyCode::K, // 45
        KeyCode::L, // 46
        KeyCode::SemiColon, // 47
        KeyCode::SingleQuote, // 48
        KeyCode::Unknown, // 49
        KeyCode::Unknown, // 50
        KeyCode::Backslash, // 51
        KeyCode::Z, // 52
        KeyCode::X, // 53
        KeyCode::C, // 54
        KeyCode::V, // 55
        KeyCode::B, // 56
        KeyCode::N, // 57
        KeyCode::M, // 58
        KeyCode::Comma, // 59
        KeyCode::Period, // 60
        KeyCode::Slash, // 61
        KeyCode::Unknown, // 62
        KeyCode::Unknown, // 63
        KeyCode::Unknown, // 64
        KeyCode::Unknown, // 65
        KeyCode::Unknown, // 66
        KeyCode::Unknown, // 67
        KeyCode::Unknown, // 68
        KeyCode::Unknown, // 69
        KeyCode::Unknown, // 70
        KeyCode::Unknown, // 71
        KeyCode::Unknown, // 72
        KeyCode::Unknown, // 73
        KeyCode::Unknown, // 74
        KeyCode::Unknown, // 75
        KeyCode::Unknown, // 76
        KeyCode::Unknown, // 77
        KeyCode::Unknown, // 78
        KeyCode::Unknown, // 79
        KeyCode::Unknown, // 80
        KeyCode::Unknown, // 81
        KeyCode::Unknown, // 82
        KeyCode::Unknown, // 83
        KeyCode::Unknown, // 84
        KeyCode::Unknown, // 85
        KeyCode::Unknown, // 86
        KeyCode::Unknown, // 87
        KeyCode::Unknown, // 88
        KeyCode::Unknown, // 89
        KeyCode::Unknown, // 90
        KeyCode::Unknown, // 91
        KeyCode::Unknown, // 92
        KeyCode::Unknown, // 93
        KeyCode::Unknown, // 94
        KeyCode::Unknown, // 95
        KeyCode::Unknown, // 96
        KeyCode::Unknown, // 97
        KeyCode::Unknown, // 98
        KeyCode::Unknown, // 99
        KeyCode::Unknown, // 100
        KeyCode::Unknown, // 101
        KeyCode::Unknown, // 102
        KeyCode::Unknown, // 103
        KeyCode::Unknown, // 104
        KeyCode::Unknown, // 105
        KeyCode::Unknown, // 106
        KeyCode::Unknown, // 107
        KeyCode::Unknown, // 108
        KeyCode::Unknown, // 109
        KeyCode::Unknown, // 110
        KeyCode::Unknown, // 111
        KeyCode::Unknown, // 112
        KeyCode::Unknown, // 113
        KeyCode::Unknown, // 114
        KeyCode::Unknown, // 115
        KeyCode::Unknown, // 116
        KeyCode::Unknown, // 117
        KeyCode::Unknown, // 118
        KeyCode::Unknown, // 119
        KeyCode::Unknown, // 120
        KeyCode::Unknown, // 121
        KeyCode::Unknown, // 122
        KeyCode::Unknown, // 123
        KeyCode::Unknown, // 124
        KeyCode::Unknown, // 125
        KeyCode::Unknown, // 126
        KeyCode::Unknown, // 127
    };

    u32 num_codes = sizeof(codes)/sizeof(KeyCode);

    if ((u32)code > num_codes)
        return KeyCode::Unknown;

    return codes[(u32)code];
}

static void update_event_queue(XcbEventQueue* q, xcb_connection_t* c)
{
    free(q->prev);
    q->prev = q->current;
    q->current = q->next;
    q->next = xcb_poll_for_event(c);
}

static bool poll_event(XcbWindow* w)
{
    update_event_queue(&w->evt_queue, w->connection);
    xcb_generic_event_t* evt = w->evt_queue.current;
 
    if (!evt)
        return false;

    // For being able to get window closed event.
    xcb_intern_atom_cookie_t window_deleted_cookie = xcb_intern_atom(w->connection, 0, 16, "WM_DELETE_WINDOW");
    xcb_intern_atom_reply_t* window_deleted_reply = xcb_intern_atom_reply(w->connection, window_deleted_cookie, 0);

    switch(evt->response_type & ~0x80)
    {
        case XCB_KEY_PRESS:
        {
            xcb_keycode_t code = ((xcb_key_press_event_t*)evt)->detail;

            KeyCode kc = xcb_key_to_keycode(code);
            
            if (kc == KeyCode::Unknown)
                info("Unknown key pressed: %d", code);

            w->state.callbacks.key_pressed_callback(kc);
            return true;
        }
        case XCB_KEY_RELEASE:
        {
            xcb_key_release_event_t* kr_evt = (xcb_key_release_event_t*)evt;
            xcb_keycode_t code = kr_evt->detail;
            
            // This (and the whole queue) is for removing "key repeat"
            if (w->evt_queue.next
                && ((w->evt_queue.next->response_type & ~0x80) == XCB_KEY_PRESS)
                && ((xcb_key_press_event_t*)w->evt_queue.next)->time == kr_evt->time
                && ((xcb_key_press_event_t*)w->evt_queue.next)->detail == code)
            {
                update_event_queue(&w->evt_queue, w->connection);
                return true;
            }

            KeyCode kc = xcb_key_to_keycode(code);
            
            if (kc == KeyCode::Unknown)
                info("Unknown key pressed: %d", code);

            w->state.callbacks.key_released_callback(kc);
            return true;
        }
        case XCB_FOCUS_OUT:
        {
            info("XCB window lost focus");
            w->state.callbacks.focus_lost_callback();
            return true;
        }
        case XCB_FOCUS_IN:
        {
            info("XCB window gained focus");
            return true;
        }
        case XCB_CLIENT_MESSAGE:
        {
            info("XCB window closed");
            if((*(xcb_client_message_event_t*)evt).data.data32[0] == (*window_deleted_reply).atom)
                w->state.open_state = WINDOW_OPEN_STATE_CLOSED;
            return true;
        }
        case XCB_CONFIGURE_NOTIFY:
        {
            xcb_configure_notify_event_t *cfg_event = (xcb_configure_notify_event_t *)evt;
            if (((cfg_event->width != w->state.width) || (cfg_event->height != w->state.height)))
            {
                    u32 width = cfg_event->width;
                    u32 height = cfg_event->height;
                    if (width > 0 && height > 0)
                    {
                        info("XCB window resized to %d x %d", width, height);
                        w->state.width = width;
                        w->state.height = height;
                        w->state.callbacks.window_resized_callback(width, height);
                    }
            }
        } return true;
        default: return true;
    }
}

void linux_xcb_window_process_all_events(XcbWindow* w)
{
    while(poll_event(w));
}

xcb_connection_t* linux_xcb_window_get_connection(XcbWindow* w)
{
    return w->connection;
}

u32 linux_xcb_window_get_handle(XcbWindow* w)
{
    return w->handle;
}

bool linux_xcb_window_is_open(XcbWindow* w)
{
    return w->state.open_state == WINDOW_OPEN_STATE_OPEN;
}

WindowState* linux_xcb_window_get_state(XcbWindow* w)
{
    return &w->state;
}