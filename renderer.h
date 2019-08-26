#pragma once

fwd_enum(WindowType);
fwd_handle(RenderResourceHandle);
fwd_handle(RenderWorldObject);
fwd_struct(Mesh);
fwd_struct(Quat);
fwd_struct(Renderer);
fwd_struct(RenderWorld);
fwd_struct(Vec3);
fwd_struct(Mat4);

Renderer* renderer_create(WindowType window_type, void* window_data);
void renderer_destroy(Renderer* rs);
RenderResourceHandle renderer_create_world(Renderer* rs);
void renderer_destroy_world(Renderer* rs, RenderResourceHandle h);
size_t renderer_world_add(Renderer* rs, RenderResourceHandle world, RenderResourceHandle mesh, const Mat4* model);
void renderer_world_remove(Renderer* rs, RenderResourceHandle world, size_t idx);
void renderer_world_move(Renderer* rs, RenderResourceHandle world, u32 idx, const Mat4* model);
RenderResourceHandle renderer_resource_load(Renderer* rs, const char* filename);
void renderer_draw_world(Renderer* rs, RenderResourceHandle pipeline_handle, RenderResourceHandle world_handle, const Vec3& cam_pos, const Quat& cam_rot);
void renderer_draw(Renderer* rs, RenderResourceHandle pipeline_handle, RenderResourceHandle mesh_handle, const Mat4& model, const Vec3& cam_pos, const Quat& cam_rot);
void renderer_present(Renderer* rs);
void renderer_update_constant_buffer(Renderer* rs, RenderResourceHandle pipeline_handle, u32 binding, void* data, u32 data_size);
void renderer_wait_for_new_frame(Renderer* rs);
void renderer_surface_resized(Renderer* rs, u32 w, u32 h);