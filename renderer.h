#pragma once

#include "window.h"

typedef struct renderer_state renderer_state_t;

renderer_state_t* renderer_init(window_type_e window_type, void* window_data);
void renderer_shutdown(renderer_state_t* rs);