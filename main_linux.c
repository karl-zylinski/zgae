#include "linux_xcb_window.h"
#include "window_state.h"

int main()
{
    linux_xcb_window_t win = {};
    linux_xcb_create_window(&win, "ZGAE", 800, 600);

    while (win.state.open_state == WINDOW_OPEN)
    {
        linux_xcb_process_all_events(&win);
    }    

    return 0;
}