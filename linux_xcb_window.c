#include "linux_xcb_window.h"
#include <xcb/xcb.h>
#include <stdlib.h>
#include <stdint.h>

void linux_xcb_create_window(linux_xcb_window_t* w, const char* title, uint32_t width, uint32_t height)
{
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
        0, 0, 640, 480,
        10,
        XCB_WINDOW_CLASS_INPUT_OUTPUT,
        screen->root_visual,
        mask, values);
    xcb_map_window(w->connection, w->handle);
    xcb_flush(w->connection);
}

void linux_xcb_process_all_events(linux_xcb_window_t* win)
{
    xcb_generic_event_t* evt;
    uint32_t run = 1;
    while (run && (evt = xcb_wait_for_event(win->connection)))
    {
        switch(evt->response_type & ~0x80)
        {
            case XCB_KEY_PRESS: {
                if (((xcb_key_press_event_t*)evt)->detail == 9)
                    run = 0;
            } break;
        }
        free(evt);
    }
}