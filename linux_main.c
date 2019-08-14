#include "linux_xcb_window.h"
#include "window.h"
#include "renderer.h"
#include "key.h"
#include "debug.h"
#include "memory.h"
#include "jzon.h"
#include "shader.h"

void key_pressed(key_t k)
{
    info("pressed: %d", (uint32_t)k);
}

void key_released(key_t k)
{
    info("pressed: %d", (uint32_t)k);
}

int main()
{
    info("Starting ZGAE");
    linux_xcb_window_t* win = linux_xcb_create_window("ZGAE", 800, 600);
    window_callbacks_t wc = {};
    wc.key_pressed_callback = &key_pressed;
    wc.key_released_callback = &key_released;
    linux_xcb_update_callbacks(win, &wc);

    renderer_state_t* renderer_state = renderer_init(WINDOW_TYPE_XCB, win);
    renderer_resource_t shader = shader_load(renderer_state, "shader_default.shader");
    (void)shader;

    info("Entering main loop");
    while (linux_xcb_is_window_open(win))
    {
        linux_xcb_process_all_events(win);
    }

    info("Main loop exited, shutting down");
    renderer_shutdown(renderer_state);
    linux_xcb_destroy_window(win);
    memory_check_leaks();
    info("Shutdown finished");
    return 0;
}