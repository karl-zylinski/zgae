#pragma once

fwd_handle(renderer_resource_handle_t);
fwd_enum(window_type_t);
fwd_struct(renderer_state_t);
fwd_struct(shader_intermediate_t);
fwd_struct(pipeline_intermediate_t);

renderer_state_t* renderer_create(window_type_t window_type, void* window_data);
void renderer_destroy(renderer_state_t* rs);
renderer_resource_handle_t renderer_load_shader(renderer_state_t* rs, const shader_intermediate_t* si);
renderer_resource_handle_t renderer_load_pipeline(renderer_state_t* rs, const pipeline_intermediate_t* si);