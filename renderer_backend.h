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

void renderer_backend_init(WindowType window_type, void* window_data);
void renderer_backend_shutdown();

RenderBackendShader* renderer_backend_create_shader(char* source, u32 source_size);

RenderBackendPipeline* renderer_backend_create_pipeline(
    const RenderBackendShader* const* shader_stages, const ShaderType* shader_stages_types, u32 shader_stages_num,
    const ShaderDataType* vertex_input_types, u32 vertex_input_types_num,
    const u32* constant_buffer_sizes, const u32* constant_buffer_binding_indices, u32 constant_buffers_num,
    const u32* push_constant_sizes, const ShaderType* push_constant_shader_types, u32 push_contants_num);

RenderBackendMesh* renderer_backend_create_mesh(Mesh* mesh);

void renderer_backend_destroy_shader(RenderBackendShader* s);
void renderer_backend_destroy_pipeline(RenderBackendPipeline* p);
void renderer_backend_destroy_mesh(RenderBackendMesh* g);

void renderer_backend_begin_frame(RenderBackendPipeline* pipeline);
void renderer_backend_draw(RenderBackendPipeline* pipeline, RenderBackendMesh* mesh, const Mat4& mvp, const Mat4& model);
void renderer_backend_end_frame();
void renderer_backend_present();

void renderer_backend_update_constant_buffer(const RenderBackendPipeline& pipeline, u32 binding, const void* data, u32 data_size, u32 offset);
void renderer_backend_wait_for_new_frame();
void renderer_backend_wait_until_idle();
void renderer_backend_surface_resized(u32 width, u32 height);
Vec2u renderer_backend_get_size();