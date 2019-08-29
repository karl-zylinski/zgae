#pragma once

fwd_struct(XcbWindow);
fwd_struct(xcb_connection_t);
fwd_struct(WindowCallbacks);
fwd_struct(WindowState);

XcbWindow* linux_xcb_window_create(char* title, u32 width, u32 height);
void linux_xcb_window_destroy(XcbWindow* w);
void linux_xcb_window_process_all_events(XcbWindow* w);
xcb_connection_t* linux_xcb_window_get_connection(XcbWindow* w);
u32 linux_xcb_window_get_handle(XcbWindow* w);
void linux_xcb_window_update_callbacks(XcbWindow* w, WindowCallbacks* wc);
bool linux_xcb_window_is_open(XcbWindow* w);
WindowState* linux_xcb_window_get_state(XcbWindow* w);