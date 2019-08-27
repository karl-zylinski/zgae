#pragma once

fwd_enum(WindowType);
fwd_handle(RenderResourceHandle);
fwd_struct(Quat);
fwd_struct(Renderer);
fwd_struct(Vec3);
fwd_struct(Mat4);

Renderer* renderer_create(WindowType window_type, void* window_data);
void renderer_destroy(mut Renderer* rs);
RenderResourceHandle renderer_create_world(mut Renderer* rs);
void renderer_destroy_world(mut Renderer* rs, RenderResourceHandle h);
size_t renderer_world_add(mut Renderer* rs, RenderResourceHandle world, RenderResourceHandle mesh, Mat4* model);
void renderer_world_remove(mut Renderer* rs, RenderResourceHandle world, size_t idx);
void renderer_world_move(mut Renderer* rs, RenderResourceHandle world, u32 idx, Mat4* model);
RenderResourceHandle renderer_resource_load(mut Renderer* rs, char* filename);
void renderer_draw_world(Renderer* rs, RenderResourceHandle pipeline_handle, RenderResourceHandle world_handle, Vec3* cam_pos, Quat* cam_rot);
void renderer_draw(Renderer* rs, RenderResourceHandle pipeline_handle, RenderResourceHandle mesh_handle, Mat4* model, Vec3* cam_pos, Quat* cam_rot);
void renderer_present(Renderer* rs);
void renderer_update_constant_buffer(Renderer* rs, RenderResourceHandle pipeline_handle, u32 binding, void* data, u32 data_size);
void renderer_wait_for_new_frame(Renderer* rs);
void renderer_surface_resized(mut Renderer* rs, u32 w, u32 h);