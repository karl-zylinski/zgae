#pragma once

fwd_handle(renderer_resource_handle_t);
fwd_enum(window_type_t);
fwd_struct(renderer_backend_state_t);
fwd_struct(shader_intermediate_t);
fwd_struct(pipeline_intermediate_t);
fwd_struct(geometry_vertex_t);
fwd_struct(renderer_backend_shader_t);
fwd_struct(renderer_backend_pipeline_t);
fwd_struct(renderer_backend_geometry_t);
fwd_enum(renderer_resource_type_t);
fwd_enum(shader_type_t);
fwd_enum(shader_data_type_t);

renderer_backend_state_t* renderer_backend_create(window_type_t window_type, void* window_data);
void renderer_backend_destroy(renderer_backend_state_t* rbs);
renderer_backend_shader_t* renderer_backend_load_shader(renderer_backend_state_t* rbs, const char* source, uint32_t source_size);

renderer_backend_pipeline_t* renderer_backend_load_pipeline(renderer_backend_state_t* rbs,
    renderer_backend_shader_t** shader_stages, shader_type_t* shader_stages_types, uint32_t shader_stages_num,
    shader_data_type_t* vertex_input_types, uint32_t vertex_input_types_num,
    uint32_t* constant_buffer_sizes, uint32_t* constant_buffer_binding_indices, uint32_t constant_buffers_num);

renderer_backend_geometry_t* renderer_backend_load_geometry(renderer_backend_state_t* rbs, const geometry_vertex_t* vertices, uint32_t vertices_num);

void renderer_backend_destroy_shader(renderer_backend_state_t* rbs, renderer_backend_shader_t* s);
void renderer_backend_destroy_pipeline(renderer_backend_state_t* rbs, renderer_backend_pipeline_t* p);
void renderer_backend_destroy_geometry(renderer_backend_state_t* rbs, renderer_backend_geometry_t* g);

void renderer_backend_draw(renderer_backend_state_t* rbs, renderer_backend_pipeline_t* pipeline, renderer_backend_geometry_t* geometry);
void renderer_backend_present(renderer_backend_state_t* rbs);
void renderer_backend_update_constant_buffer(renderer_backend_state_t* rbs, void* pipeline_state, uint32_t binding, void* data, uint32_t data_size);
void renderer_backend_wait_for_new_frame(renderer_backend_state_t* rbs);
void renderer_backend_wait_until_idle(renderer_backend_state_t* rbs);