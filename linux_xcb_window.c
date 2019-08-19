#include "linux_xcb_window.h"
#include <xcb/xcb.h>
#include <string.h>
#include "debug.h"
#include "keycode_types.h"
#include "window_types.h"
#include "memory.h"
#include <stdlib.h>

typedef struct xcb_event_queue_t {
    xcb_generic_event_t* prev;
    xcb_generic_event_t* current;
    xcb_generic_event_t* next;
} xcb_event_queue_t;

typedef struct linux_xcb_window_t {
    xcb_connection_t* connection;
    xcb_event_queue_t evt_queue;
    uint32_t handle;
    window_state_t state;
} linux_xcb_window_t;

linux_xcb_window_t* linux_xcb_window_create(const char* title, uint32_t width, uint32_t height)
{
    linux_xcb_window_t* w = mema_zero(sizeof(linux_xcb_window_t));
    info("Creating XCB window w/ title %s, width %d, height %d", title, width, height);
    w->connection = xcb_connect(NULL, NULL);
    xcb_screen_t* screen = xcb_setup_roots_iterator(xcb_get_setup(w->connection)).data;
    w->handle = xcb_generate_id(w->connection);
    uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32_t values[] = { screen->black_pixel, 
        XCB_EVENT_MASK_EXPOSURE
        | XCB_EVENT_MASK_KEY_PRESS
        | XCB_EVENT_MASK_KEY_RELEASE
        | XCB_EVENT_MASK_FOCUS_CHANGE };
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

void linux_xcb_window_destroy(linux_xcb_window_t* w)
{
    info("Destroying XCB window");
    xcb_destroy_window(w->connection, w->handle);
    xcb_disconnect(w->connection);
    memf(w);
}

void linux_xcb_window_update_callbacks(linux_xcb_window_t* w, const window_callbacks_t* wc)
{
    w->state.callbacks = *wc;
}

static keycode_t xcb_key_to_keycode(xcb_keycode_t code)
{
    static keycode_t codes[] = {
        KC_UNKNOWN, // 0
        KC_UNKNOWN, // 1
        KC_UNKNOWN, // 2
        KC_UNKNOWN, // 3
        KC_UNKNOWN, // 4
        KC_UNKNOWN, // 5
        KC_UNKNOWN, // 6
        KC_UNKNOWN, // 7
        KC_UNKNOWN, // 8
        KC_UNKNOWN, // 9
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

    uint32_t num_codes = sizeof(codes)/sizeof(keycode_t);

    if ((uint32_t)code > num_codes || codes[(uint32_t)code] == KC_UNKNOWN)
    {
        info("Unkown key pressed: %d", code);
        return KC_UNKNOWN;
    }

    return codes[(uint32_t)code];
}

static void update_event_queue(xcb_event_queue_t* q, xcb_connection_t* c)
{
    free(q->prev);
    q->prev = q->current;
    q->current = q->next;
    q->next = xcb_poll_for_event(c);
}

static bool poll_event(linux_xcb_window_t* w)
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
            w->state.callbacks.key_pressed_callback(xcb_key_to_keycode(code));
            return true;
        }
        case XCB_KEY_RELEASE:
        {
            xcb_key_release_event_t* kr_evt = (xcb_key_release_event_t*)evt;
            xcb_keycode_t code = kr_evt->detail;
            
            if (w->evt_queue.next
                && ((w->evt_queue.next->response_type & ~0x80) == XCB_KEY_PRESS)
                && ((xcb_key_press_event_t*)w->evt_queue.next)->time == kr_evt->time
                && ((xcb_key_press_event_t*)w->evt_queue.next)->detail == code)
            {
                update_event_queue(&w->evt_queue, w->connection);
                return true;
            }

            w->state.callbacks.key_released_callback(xcb_key_to_keycode(code));
            return true;
        }
        case XCB_FOCUS_OUT:
        {
            w->state.callbacks.focus_lost_callback();
            return true;
        }
        case XCB_CLIENT_MESSAGE:
        {
            info("XCB window closed");
            if((*(xcb_client_message_event_t*)evt).data.data32[0] == (*window_deleted_reply).atom)
                w->state.open_state = WINDOW_OPEN_STATE_CLOSED;
            return true;
        }
        default: return true;
    }
}

void linux_xcb_window_process_all_events(linux_xcb_window_t* w)
{
    while(poll_event(w));
}

xcb_connection_t* linux_xcb_window_get_connection(linux_xcb_window_t* w)
{
    return w->connection;
}

uint32_t linux_xcb_window_get_handle(linux_xcb_window_t* w)
{
    return w->handle;
}

int linux_xcb_window_is_open(linux_xcb_window_t* w)
{
    return w->state.open_state == WINDOW_OPEN_STATE_OPEN;
}