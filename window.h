#pragma once

fwd_enum(keycode_t);
typedef void(*window_key_pressed_callback_t)(keycode_t k);
typedef void(*window_key_released_callback_t)(keycode_t k);

typedef enum window_type_t {
    WINDOW_TYPE_XCB
} window_type_t;

typedef enum window_open_state_t {
    WINDOW_OPEN_STATE_OPEN, WINDOW_OPEN_STATE_CLOSED
} window_open_state_t;

typedef struct window_callbacks_t {
    window_key_pressed_callback_t key_pressed_callback;
    window_key_released_callback_t key_released_callback;
} window_callbacks_t;

typedef struct window_state_t {
    window_open_state_t open_state;
    window_callbacks_t callbacks;
} window_state_t;