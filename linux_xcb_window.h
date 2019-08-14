#pragma once

fwd_struct(linux_xcb_window_t);
fwd_struct(xcb_connection_t);
fwd_struct(window_callbacks_t);

linux_xcb_window_t* linux_xcb_window_create(const char* title, uint32_t width, uint32_t height);
void linux_xcb_window_destroy(linux_xcb_window_t* w);
void linux_xcb_window_process_all_events(linux_xcb_window_t* w);
xcb_connection_t* linux_xcb_window_get_connection(linux_xcb_window_t* w);
uint32_t linux_xcb_window_get_handle(linux_xcb_window_t* w);
void linux_xcb_window_update_callbacks(linux_xcb_window_t* w, const window_callbacks_t* wc);
int linux_xcb_window_is_open(linux_xcb_window_t* w);