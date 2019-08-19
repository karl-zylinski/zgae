#pragma once

fwd_handle(renderer_resource_handle_t);
fwd_enum(window_type_t);
fwd_struct(renderer_backend_state_t);
fwd_struct(shader_intermediate_t);
fwd_struct(pipeline_intermediate_t);
fwd_struct(geometry_vertex_t);
fwd_enum(renderer_resource_type_t);

renderer_backend_state_t* renderer_backend_create(window_type_t window_type, void* window_data);
void renderer_backend_destroy(renderer_backend_state_t* rbs);
void renderer_backend_destroy_resource(renderer_backend_state_t* rbs, renderer_resource_type_t type, void* state);
void* renderer_backend_load_shader(renderer_backend_state_t* rbs, const shader_intermediate_t* si);
void* renderer_backend_load_pipeline(renderer_backend_state_t* rbs, void** shader_stages, uint32_t shader_stages_count);
void* renderer_backend_load_geometry(renderer_backend_state_t* rbs, const geometry_vertex_t* vertices, uint32_t vertices_num);
void renderer_backend_draw(renderer_backend_state_t* rbs, void* pipeline_state, void* geometry_state);
void renderer_backend_present(renderer_backend_state_t* rbs);
void renderer_backend_update_constant_buffer(renderer_backend_state_t* rbs, void* pipeline_state, uint32_t binding, void* data, uint32_t data_size);
void renderer_backend_wait_for_new_frame(renderer_backend_state_t* rbs);