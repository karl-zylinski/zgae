#pragma once
#include "renderer_types.h"

fwd_enum(WindowType);
fwd_handle(RenderResourceHandle);
fwd_struct(Quat);
fwd_struct(Renderer);
fwd_struct(Vec3);
fwd_struct(Mat4);

void renderer_init(WindowType window_type, void* window_data);
void renderer_shutdown();
RenderResourceHandle renderer_create_world();
void renderer_destroy_world(RenderResourceHandle h);
RenderWorldObjectHandle renderer_world_add(RenderResourceHandle world, RenderResourceHandle mesh, const Vec3& position, const Quat& rot);
void renderer_world_remove(RenderResourceHandle world, RenderWorldObjectHandle h);
void renderer_world_set_position_and_rotation(RenderResourceHandle world, RenderWorldObjectHandle h, const Vec3& position, const Quat& rot);
RenderResourceHandle renderer_resource_load(const char* filename);
void renderer_begin_frame(RenderResourceHandle pipeline_handle);
void renderer_draw_world(RenderResourceHandle pipeline_handle, RenderResourceHandle world_handle, const Vec3& cam_pos, const Quat& cam_rot);
void renderer_end_frame();
void renderer_draw(RenderResourceHandle pipeline_handle, RenderResourceHandle mesh_handle, const Mat4& model, const Vec3& cam_pos, const Quat& cam_rot);
void renderer_present();
void renderer_update_constant_buffer(RenderResourceHandle pipeline_handle, u32 binding, void* data, u32 data_size);
void renderer_wait_for_new_frame();
void renderer_surface_resized(u32 w, u32 h);