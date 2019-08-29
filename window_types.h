#pragma once

fwd_enum(KeyCode);
typedef void(*WindowKeyPressedCallback)(KeyCode k);
typedef void(*WindowKeyReleasedCallback)(KeyCode k);
typedef void(*WindowResizedCallback)(u32 w, u32 h);
typedef void(*WindowFocusLostCallback)();

enum WindowType : u32
{
    WINDOW_TYPE_XCB
};

enum WindowOpenState
{
    WINDOW_OPEN_STATE_OPEN, WINDOW_OPEN_STATE_CLOSED
};

struct WindowCallbacks {
    WindowKeyPressedCallback key_pressed_callback;
    WindowKeyReleasedCallback key_released_callback;
    WindowFocusLostCallback focus_lost_callback;
    WindowResizedCallback window_resized_callback;
};

struct WindowState {
    WindowOpenState open_state;
    WindowCallbacks callbacks;
    u32 width;
    u32 height;
};