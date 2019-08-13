#pragma once

enum key;
typedef void(*window_key_pressed_callback_t)(enum key);
typedef void(*window_key_released_callback_t)(enum key);

enum window_type {
    WINDOW_TYPE_XCB
};

enum window_open_state {
    WINDOW_OPEN_STATE_OPEN, WINDOW_OPEN_STATE_CLOSED
};

struct window_callbacks {
    window_key_pressed_callback_t key_pressed_callback;
    window_key_released_callback_t key_released_callback;
};

struct window_state {
    enum window_open_state open_state;
    struct window_callbacks callbacks;
};