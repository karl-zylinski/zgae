#pragma once

fwd_struct(XcbWindow);
fwd_struct(xcb_connection_t);
fwd_struct(WindowCallbacks);
fwd_struct(WindowState);

XcbWindow* linux_xcb_window_create(const char* title, u32 width, u32 height);
void linux_xcb_window_destroy(XcbWindow* w);
void linux_xcb_window_process_all_events(XcbWindow* w);
xcb_connection_t* linux_xcb_window_get_connection(const XcbWindow* w);
u32 linux_xcb_window_get_handle(const XcbWindow* w);
void linux_xcb_window_update_callbacks(XcbWindow* w, const WindowCallbacks* wc);
bool linux_xcb_window_is_open(const XcbWindow* w);
const WindowState* linux_xcb_window_get_state(const XcbWindow* w);