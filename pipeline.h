#pragma once
#include "pipeline_types.h"

fwd_struct(renderer_state_t);

renderer_resource_handle_t pipeline_load(renderer_state_t* rs, const char* filename);