#include "linux_xcb_window.h"
#include "renderer.h"
#include "key.h"
#include <stdio.h>
#include "debug.h"
#include "memory.h"

void key_pressed(key_t key)
{
    info("pressed: %d", (uint32_t)key);
}

void key_released(key_t key)
{
    info("pressed: %d", (uint32_t)key);
}

int main()
{
    info("Starting ZGAE");
    linux_xcb_window_t win = {0};
    win.state.key_pressed_callback = &key_pressed;
    win.state.key_released_callback = &key_released;
    linux_xcb_create_window(&win, "ZGAE", 800, 600);

    renderer_state_t* renderer_state = renderer_init(WINDOW_TYPE_XCB, &win);

    info("Entering main loop");
    while (win.state.open_state == WINDOW_OPEN_STATE_OPEN)
    {
        linux_xcb_process_all_events(&win);
    }

    info("Main loop exited, shutting down");
    renderer_shutdown(renderer_state);
    memory_check_leaks();
    info("Shutdown finished");
    return 0;
}