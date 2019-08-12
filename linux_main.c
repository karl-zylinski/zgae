#include "linux_xcb_window.h"
#include "renderer.h"
#include "key.h"
#include <stdio.h>
#include "debug.h"
#include "memory.h"
#include "jzon.h"
#include "file.h"

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
    char* data;
    size_t data_size;
    file_load_str("shader_default.shader", &data, &data_size);
    jzon_value_t jzon_res;
    int parse_success = jzon_parse(data, &jzon_res);
    check(parse_success == 1, "lax");

    info("Starting ZGAE");
    linux_xcb_window_t win = {};
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