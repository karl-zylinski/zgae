#pragma once

fwd_struct(Quat);
fwd_struct(Vec3);
fwd_struct(Vec4);
fwd_struct(Mat4);
fwd_enum(PrimitiveTopology);
fwd_struct(RenderWorld);

struct GenericWindowInfo
{
    void* display;
    u64 handle;
};

enum WindowType : u32
{
    WINDOW_TYPE_X11
};

void renderer_init(WindowType window_type, const GenericWindowInfo& window_data);
void renderer_shutdown();
RenderWorld* renderer_create_world();
void renderer_destroy_world(RenderWorld* w);
u32 renderer_create_object(RenderWorld* w, u32 mesh_idx, const Vec3& position, const Quat& rot);
void renderer_destroy_object(RenderWorld* w, u32 object_idx);
void renderer_world_set_position_and_rotation(RenderWorld* w, u32 object_idx, const Vec3& position, const Quat& rot);
u32 renderer_load_mesh(const char* filename);
void renderer_destroy_mesh(u32 mesh_idx);
u32 renderer_load_pipeline(const char* filename);
void renderer_destroy_pipeline(u32 pipeline_idx);
void renderer_begin_frame(u32 pipeline_idx);
void renderer_draw_world(u32 pipeline_idx, RenderWorld* w, const Vec3& cam_pos, const Quat& cam_rot);
void renderer_draw(u32 pipeline_idx, u32 mesh_idx, const Mat4& model, const Vec3& cam_pos, const Quat& cam_rot);
void renderer_present();
void renderer_update_constant_buffer(u32 pipeline_idx, u32 binding, void* data, u32 data_size);
void renderer_surface_resized(u32 w, u32 h);
void renderer_debug_draw(const Vec3* vertices, u32 vertices_num, const Vec4* colors, PrimitiveTopology topology, const Vec3& cam_pos, const Quat& cam_rot);