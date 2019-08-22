#pragma once

#include "geometry_types.h"

fwd_handle(renderer_resource_handle_t);
fwd_enum(window_type_t);
fwd_struct(renderer_state_t);
fwd_struct(shader_intermediate_t);
fwd_struct(pipeline_intermediate_t);

renderer_state_t* renderer_create(window_type_t window_type, void* window_data);
void renderer_destroy(renderer_state_t* rs);
renderer_resource_handle_t renderer_load_shader(renderer_state_t* rs, const shader_intermediate_t* si);
renderer_resource_handle_t renderer_load_pipeline(renderer_state_t* rs, const pipeline_intermediate_t* pi);
renderer_resource_handle_t renderer_load_geometry(renderer_state_t* rs, const geometry_vertex_t* vertices, uint32_t vertices_num, const geometry_index_t* indices, uint32_t indices_num);
void renderer_draw(renderer_state_t* rs, renderer_resource_handle_t pipeline_handle, renderer_resource_handle_t geometry_handle);
void renderer_present(renderer_state_t* rs);
void renderer_update_constant_buffer(renderer_state_t* rs, renderer_resource_handle_t pipeline_handle, uint32_t binding, void* data, uint32_t data_size);
void renderer_wait_for_new_frame(renderer_state_t* rs);
void renderer_surface_resized(renderer_state_t* rs, uint32_t w, uint32_t h);