#pragma once

#include "key.h"

typedef void(*window_key_pressed_callback_t)(key_t key);
typedef void(*window_key_released_callback_t)(key_t key);

typedef enum {
    WINDOW_TYPE_XCB
} window_type_t;

typedef enum {
    WINDOW_OPEN_STATE_OPEN, WINDOW_OPEN_STATE_CLOSED
} window_open_state_t;

typedef struct {
    window_open_state_t open_state;
    window_key_pressed_callback_t key_pressed_callback;
    window_key_released_callback_t key_released_callback;
} window_state_t;
