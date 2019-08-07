#pragma once

typedef enum {
    WINDOW_TYPE_XCB
} window_type_e;

typedef enum {
    WINDOW_OPEN_STATE_OPEN, WINDOW_OPEN_STATE_CLOSED
} window_open_state_e;

typedef struct {
    window_open_state_e open_state;
} window_state_t;
