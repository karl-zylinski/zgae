#include "linux_xcb_window.h"
#include "renderer.h"
#include "key.h"
#include <stdio.h>

void key_pressed(key_e key)
{
    printf("pressed: %d\n", (uint32_t)key);
}

void key_released(key_e key)
{
    printf("released: %d\n", (uint32_t)key);
}

int main()
{
    linux_xcb_window_t win = {};
    win.state.key_pressed_callback = &key_pressed;
    win.state.key_released_callback = &key_released;
    linux_xcb_create_window(&win, "ZGAE", 800, 600);

    renderer_state_t* renderer_state = renderer_init(WINDOW_TYPE_XCB, &win);

    while (win.state.open_state == WINDOW_OPEN_STATE_OPEN)
    {
        linux_xcb_process_all_events(&win);
    }

    renderer_shutdown(renderer_state);

    return 0;
}