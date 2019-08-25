#pragma once

fwd_handle(RendererResourceHandle);
fwd_enum(WindowType);
fwd_struct(RendererState);
fwd_struct(Mesh);
fwd_struct(Vec3);
fwd_struct(Quat);
fwd_handle(ResourceHandle);

RendererState* renderer_create(WindowType window_type, void* window_data);
void renderer_destroy(RendererState* rs);
RendererResourceHandle renderer_resource_load(RendererState* rs, const char* filename);
void renderer_draw(RendererState* rs, RendererResourceHandle pipeline_handle, RendererResourceHandle mesh_handle, const Vec3* cam_pos, const Quat* cam_rot);
void renderer_present(RendererState* rs);
void renderer_update_constant_buffer(RendererState* rs, RendererResourceHandle pipeline_handle, u32 binding, void* data, u32 data_size);
void renderer_wait_for_new_frame(RendererState* rs);
void renderer_surface_resized(RendererState* rs, u32 w, u32 h);