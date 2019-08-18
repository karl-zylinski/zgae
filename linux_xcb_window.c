#include "linux_xcb_window.h"
#include <xcb/xcb.h>
#include <string.h>
#include "debug.h"
#include "key_types.h"
#include "window.h"
#include "memory.h"
#include <stdlib.h>

typedef struct linux_xcb_window_t {
    xcb_connection_t* connection;
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
    uint32_t values[] = {screen->black_pixel,  XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS};
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
    (void)w;
    error("Please implement!!");
}

void linux_xcb_window_update_callbacks(linux_xcb_window_t* w, const window_callbacks_t* wc)
{
    w->state.callbacks = *wc;
}

void linux_xcb_window_process_all_events(linux_xcb_window_t* w)
{
    // For being able to get window closed event.
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
                info("XCB window closed");
                if((*(xcb_client_message_event_t*)evt).data.data32[0] == (*window_deleted_reply).atom)
                    w->state.open_state = WINDOW_OPEN_STATE_CLOSED;
            } break;
            default: break;
        }
        free(evt);
    }
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