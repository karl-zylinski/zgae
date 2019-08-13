#pragma once

#include <stdint.h>

struct linux_xcb_window;
struct xcb_connection_t;
struct window_callbacks;

struct linux_xcb_window* linux_xcb_create_window(const char* title, uint32_t width, uint32_t height);
void linux_xcb_process_all_events(struct linux_xcb_window* win);
struct xcb_connection_t* linux_xcb_get_connection(struct linux_xcb_window* w);
uint32_t linux_xcb_get_window_handle(struct linux_xcb_window* w);
void linux_xcb_update_callbacks(struct linux_xcb_window* w, const struct window_callbacks* wc);
int linux_xcb_is_window_open(struct linux_xcb_window* w);