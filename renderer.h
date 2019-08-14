#pragma once

fwd_enum(window_type_t);
fwd_struct(renderer_state_t);

renderer_state_t* renderer_init(window_type_t window_type, void* window_data);
void renderer_shutdown(renderer_state_t* rs);
//void renderer_load_shader(renderer_state_t* rs, const shader_intermediate_t* si);