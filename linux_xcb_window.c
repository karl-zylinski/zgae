#include "linux_xcb_window.h"
#include <xcb/xcb.h>
#include <string.h>
#include "debug.h"
#include "key.h"
#include "window.h"
#include "memory.h"
#include <stdlib.h>

struct linux_xcb_window {
    struct xcb_connection_t* connection;
    uint32 handle;
    struct window_state state;
};

struct linux_xcb_window* linux_xcb_create_window(const char* title, uint32 width, uint32 height)
{
    struct linux_xcb_window* w = mema_zero(sizeof(struct linux_xcb_window));
    info("Creating XCB window w/ title %s, width %d, height %d", title, width, height);
    w->connection = xcb_connect(NULL, NULL);
    xcb_screen_t* screen = xcb_setup_roots_iterator(xcb_get_setup(w->connection)).data;
    w->handle = xcb_generate_id(w->connection);

    uint32 mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
    uint32 values[] = {screen->black_pixel,  XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS};
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

    xcb_change_property(
        w->connection,
        XCB_PROP_MODE_REPLACE,
        w->handle,
        XCB_ATOM_WM_NAME,
        XCB_ATOM_STRING,
        8,
        strlen(title),
        title);

    xcb_intern_atom_cookie_t cookie = xcb_intern_atom(w->connection, 1, 12, "WM_PROTOCOLS");
    xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(w->connection, cookie, 0);

    xcb_intern_atom_cookie_t cookie2 = xcb_intern_atom(w->connection, 0, 16, "WM_DELETE_WINDOW");
    xcb_intern_atom_reply_t* reply2 = xcb_intern_atom_reply(w->connection, cookie2, 0);

    xcb_change_property(w->connection, XCB_PROP_MODE_REPLACE, w->handle, (*reply).atom, 4, 32, 1, &(*reply2).atom);

    xcb_flush(w->connection);
    return w;
}

void linux_xcb_destroy_window(struct linux_xcb_window* w)
{
    (void)w;
    error("Please implement.");
}

void linux_xcb_update_callbacks(struct linux_xcb_window* w, const struct window_callbacks* wc)
{
    w->state.callbacks = *wc;
}

void linux_xcb_process_all_events(struct linux_xcb_window* w)
{
    xcb_intern_atom_cookie_t window_deleted_cookie = xcb_intern_atom(w->connection, 0, 16, "WM_DELETE_WINDOW");
    xcb_intern_atom_reply_t* window_deleted_reply = xcb_intern_atom_reply(w->connection, window_deleted_cookie, 0);

    xcb_generic_event_t* evt;
    while ((evt = xcb_poll_for_event(w->connection)))
    {
        switch(evt->response_type & ~0x80)
        {
            case XCB_KEY_PRESS: {
                xcb_keycode_t code = ((xcb_key_press_event_t*)evt)->detail;
                w->state.callbacks.key_pressed_callback(code);
            } break;
            case XCB_KEY_RELEASE: { // This is broken, needs poooop xcb crap. I just want to know when the actual key goes up, nothing else...
                xcb_keycode_t code = ((xcb_key_release_event_t*)evt)->detail;
                w->state.callbacks.key_released_callback(code);
            } break;
            case XCB_CLIENT_MESSAGE:
            {
                info("XCB got message to close application");
                if((*(xcb_client_message_event_t*)evt).data.data32[0] == (*window_deleted_reply).atom)
                    w->state.open_state = WINDOW_OPEN_STATE_CLOSED;
            } break;
            default: break;
        }
        free(evt);
    }
}

struct xcb_connection_t* linux_xcb_get_connection(struct linux_xcb_window* w)
{
    return w->connection;
}

uint32 linux_xcb_get_window_handle(struct linux_xcb_window* w)
{
    return w->handle;
}

int linux_xcb_is_window_open(struct linux_xcb_window* w)
{
    return w->state.open_state == WINDOW_OPEN_STATE_OPEN;
}