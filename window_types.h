#pragma once

fwd_enum(KeyCode);
typedef void(*WindowKeyPressedCallback)(KeyCode k);
typedef void(*WindowKeyReleasedCallback)(KeyCode k);
typedef void(*WindowResizedCallback)(u32 w, u32 h);
typedef void(*WindowFocusLostCallback)();

typedef enum WindowType {
    WINDOW_TYPE_XCB
} WindowType;

typedef enum WindowOpenState {
    WINDOW_OPEN_STATE_OPEN, WINDOW_OPEN_STATE_CLOSED
} WindowOpenState;

typedef struct WindowCallbacks {
    WindowKeyPressedCallback key_pressed_callback;
    WindowKeyReleasedCallback key_released_callback;
    WindowFocusLostCallback focus_lost_callback;
    WindowResizedCallback window_resized_callback;
} WindowCallbacks;

typedef struct WindowState {
    WindowOpenState open_state;
    WindowCallbacks callbacks;
    u32 width;
    u32 height;
} WindowState;