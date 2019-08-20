#pragma once

fwd_enum(keycode_t);
typedef void(*window_key_pressed_callback_t)(keycode_t k);
typedef void(*window_key_released_callback_t)(keycode_t k);
typedef void(*window_resized_callback_t)(uint32_t w, uint32_t h);
typedef void(*window_focus_lost_callback_t)();

typedef enum window_type_t {
    WINDOW_TYPE_XCB
} window_type_t;

typedef enum window_open_state_t {
    WINDOW_OPEN_STATE_OPEN, WINDOW_OPEN_STATE_CLOSED
} window_open_state_t;

typedef struct window_callbacks_t {
    window_key_pressed_callback_t key_pressed_callback;
    window_key_released_callback_t key_released_callback;
    window_focus_lost_callback_t focus_lost_callback;
    window_resized_callback_t window_resized_callback;
} window_callbacks_t;

typedef struct window_state_t {
    window_open_state_t open_state;
    window_callbacks_t callbacks;
    uint32_t width;
    uint32_t height;
} window_state_t;