#pragma once

fwd_enum(WindowType);
fwd_handle(RenderResourceHandle);
fwd_handle(RenderWorldObject);
fwd_struct(Mesh);
fwd_struct(Quat);
fwd_struct(RendererState);
fwd_struct(RenderWorld);
fwd_struct(Vec3);
fwd_struct(Mat4);

RendererState* renderer_create(WindowType window_type, void* window_data);
void renderer_destroy(RendererState* rs);
RenderResourceHandle renderer_create_world(RendererState* rs);
void renderer_destroy_world(RendererState* rs, RenderResourceHandle h);
size_t renderer_world_add(RendererState* rs, RenderResourceHandle world, RenderResourceHandle obj);
void renderer_world_remove(RendererState* rs, RenderResourceHandle world, size_t idx);
RenderResourceHandle renderer_resource_load(RendererState* rs, const char* filename);
void renderer_draw_world(RendererState* rs, RenderResourceHandle pipeline_handle, RenderResourceHandle world_handle, const Vec3* cam_pos, const Quat* cam_rot);
void renderer_draw(RendererState* rs, RenderResourceHandle pipeline_handle, RenderResourceHandle mesh_handle, const Mat4* model, const Vec3* cam_pos, const Quat* cam_rot);
void renderer_present(RendererState* rs);
void renderer_update_constant_buffer(RendererState* rs, RenderResourceHandle pipeline_handle, u32 binding, void* data, u32 data_size);
void renderer_wait_for_new_frame(RendererState* rs);
void renderer_surface_resized(RendererState* rs, u32 w, u32 h);