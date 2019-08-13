#include "linux_xcb_window.h"
#include "window.h"
#include "renderer.h"
#include "key.h"
#include <stdio.h>
#include "debug.h"
#include "memory.h"
#include "jzon.h"
#include "file.h"
#include "shader.h"

void key_pressed(enum key key)
{
    info("pressed: %d", (uint32_t)key);
}

void key_released(enum key key)
{
    info("pressed: %d", (uint32_t)key);
}

int main()
{
    info("Starting ZGAE");
    struct linux_xcb_window* win = linux_xcb_create_window("ZGAE", 800, 600);
    struct window_callbacks wc = {};
    wc.key_pressed_callback = &key_pressed;
    wc.key_released_callback = &key_released;
    linux_xcb_update_callbacks(win, &wc);

    struct renderer_state* renderer_state = renderer_init(WINDOW_TYPE_XCB, win);
    uint32_t shader = shader_load(renderer_state, "shader_default.shader");
    (void)shader;

    info("Entering main loop");
    while (linux_xcb_is_window_open(win))
    {
        linux_xcb_process_all_events(win);
    }

    info("Main loop exited, shutting down");
    renderer_shutdown(renderer_state);
    memory_check_leaks();
    info("Shutdown finished");
    return 0;
}