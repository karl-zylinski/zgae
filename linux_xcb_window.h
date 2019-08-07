#pragma once

#include <stdint.h>
#include "window_state.h"

typedef struct {
    struct xcb_connection_t* connection;
    uint32_t handle;
    window_state_t state;
} linux_xcb_window_t;

void linux_xcb_create_window(linux_xcb_window_t* w, const char* title, uint32_t width, uint32_t height);
void linux_xcb_process_all_events(linux_xcb_window_t* win);