#pragma once
#include "math_types.h"

fwd_struct(RendererBackend);
fwd_struct(RenderBackendShader);
fwd_struct(RenderBackendPipeline);
fwd_struct(RenderBackendMesh);
fwd_struct(Mesh);
fwd_enum(WindowType);
fwd_enum(ShaderType);
fwd_enum(ShaderDataType);

RendererBackend* renderer_backend_create(WindowType window_type, void* window_data);
void renderer_backend_destroy(RendererBackend* rbs);

RenderBackendShader* renderer_backend_create_shader(RendererBackend* rbs, char* source, u32 source_size);

RenderBackendPipeline* renderer_backend_create_pipeline(RendererBackend* rbs,
    RenderBackendShader** shader_stages, ShaderType* shader_stages_types, u32 shader_stages_num,
    ShaderDataType* vertex_input_types, u32 vertex_input_types_num,
    u32* constant_buffer_sizes, u32* constant_buffer_binding_f, u32 constant_buffers_num);

RenderBackendMesh* renderer_backend_create_mesh(RendererBackend* rbs, Mesh* mesh);

void renderer_backend_destroy_shader(RendererBackend* rbs, RenderBackendShader* s);
void renderer_backend_destroy_pipeline(RendererBackend* rbs, RenderBackendPipeline* p);
void renderer_backend_destroy_mesh(RendererBackend* rbs, RenderBackendMesh* g);

void renderer_backend_draw(RendererBackend* rbs, RenderBackendPipeline* pipeline, RenderBackendMesh* mesh);
void renderer_backend_present(RendererBackend* rbs);

void renderer_backend_update_constant_buffer(RendererBackend* rbs, C(RenderBackendPipeline) pipeline, u32 binding, const void* data, u32 data_size, u32 offset);
void renderer_backend_wait_for_new_frame(RendererBackend* rbs);
void renderer_backend_wait_until_idle(RendererBackend* rbs);
void renderer_backend_surface_resized(RendererBackend* rbs, u32 width, u32 height);
Vec2u renderer_backend_get_size(RendererBackend* rbs);