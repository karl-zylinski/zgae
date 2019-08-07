#pragma once

typedef enum {
    WINDOW_OPEN, WINDOW_CLOSED
} window_open_state_e;

typedef struct {
    window_open_state_e open_state;
} window_state_t;
