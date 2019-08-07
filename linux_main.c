#include "linux_xcb_window.h"
#include "renderer.h"

int main()
{
    linux_xcb_window_t win = {};
    linux_xcb_create_window(&win, "ZGAE", 800, 600);

    renderer_state_t* renderer_state = renderer_init(WINDOW_TYPE_XCB, &win);
    (void)renderer_state;

    while (win.state.open_state == WINDOW_OPEN_STATE_OPEN)
    {
        linux_xcb_process_all_events(&win);
    }

    renderer_shutdown(renderer_state);

    return 0;
}