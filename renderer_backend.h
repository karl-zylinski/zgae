#pragma once
#include "math_types.h"

fwd_struct(RendererBackendState);
fwd_struct(RendererBackendShader);
fwd_struct(RendererBackendPipeline);
fwd_struct(RendererBackendMesh);
fwd_enum(ShaderType);
fwd_enum(ShaderDataType);

RendererBackendState* renderer_backend_create(WindowType window_type, void* window_data);
void renderer_backend_destroy(RendererBackendState* rbs);

RendererBackendShader* renderer_backend_create_shader(RendererBackendState* rbs, const char* source, u32 source_size);

RendererBackendPipeline* renderer_backend_create_pipeline(RendererBackendState* rbs,
    RendererBackendShader** shader_stages, ShaderType* shader_stages_types, u32 shader_stages_num,
    ShaderDataType* vertex_input_types, u32 vertex_input_types_num,
    u32* constant_buffer_sizes, u32* constant_buffer_binding_f, u32 constant_buffers_num);

RendererBackendMesh* renderer_backend_create_mesh(RendererBackendState* rbs, const Mesh* mesh);

void renderer_backend_destroy_shader(RendererBackendState* rbs, RendererBackendShader* s);
void renderer_backend_destroy_pipeline(RendererBackendState* rbs, RendererBackendPipeline* p);
void renderer_backend_destroy_mesh(RendererBackendState* rbs, RendererBackendMesh* g);

void renderer_backend_draw(RendererBackendState* rbs, RendererBackendPipeline* pipeline, RendererBackendMesh* mesh);
void renderer_backend_present(RendererBackendState* rbs);
void renderer_backend_update_constant_buffer(RendererBackendState* rbs, void* pipeline_state, u32 binding, const void* data, u32 data_size, u32 offset);
void renderer_backend_wait_for_new_frame(RendererBackendState* rbs);
void renderer_backend_wait_until_idle(RendererBackendState* rbs);
void renderer_backend_surface_resized(RendererBackendState* rbs, u32 width, u32 height);
Vec2u renderer_backend_get_size(RendererBackendState* rbs);