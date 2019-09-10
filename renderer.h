#pragma once

fwd_handle(RenderResourceHandle);
fwd_struct(Quat);
fwd_struct(Vec3);
fwd_struct(Vec4);
fwd_struct(Mat4);
fwd_enum(PrimitiveTopology);
fwd_struct(RenderWorld);
typedef u32 RenderWorldObjectHandle;

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
RenderWorldObjectHandle renderer_create_object(RenderWorld* w, RenderResourceHandle mesh, const Vec3& position, const Quat& rot);
void renderer_destroy_object(RenderWorld* w, RenderWorldObjectHandle h);
void renderer_world_set_position_and_rotation(RenderWorld* w, RenderWorldObjectHandle h, const Vec3& position, const Quat& rot);
RenderResourceHandle renderer_load_resource(const char* filename);
void renderer_begin_frame(RenderResourceHandle pipeline_handle);
void renderer_draw_world(RenderResourceHandle pipeline_handle, RenderWorld* w, const Vec3& cam_pos, const Quat& cam_rot);
void renderer_draw(RenderResourceHandle pipeline_handle, RenderResourceHandle mesh_handle, const Mat4& model, const Vec3& cam_pos, const Quat& cam_rot);
void renderer_present();
void renderer_update_constant_buffer(RenderResourceHandle pipeline_handle, u32 binding, void* data, u32 data_size);
void renderer_surface_resized(u32 w, u32 h);
void renderer_debug_draw(const Vec3* vertices, u32 vertices_num, const Vec4* colors, PrimitiveTopology topology, const Vec3& cam_pos, const Quat& cam_rot);