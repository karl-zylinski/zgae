#pragma once
#include "render_resource.h"

struct Vertex;
struct Rect;
struct RenderWorld;
struct RenderResource;
struct RenderObject;
struct Quaternion;
struct Vector3;
struct Matrix4x4;
struct Mesh;

enum struct DrawLights { DrawLights, DoNotDrawLights };

// TODO: Remove unneeded stuff in interface.
struct Renderer
{
    virtual void init(void* window_handle) = 0;
    virtual void shutdown() = 0;
    virtual RRHandle load_shader(const char* filename) = 0;
    virtual void set_shader(RRHandle shader) = 0;
    virtual RenderTarget create_back_buffer() = 0;
    virtual RenderTarget create_render_texture(PixelFormat pf, unsigned width, unsigned height) = 0;
    virtual unsigned find_free_resource_handle() const = 0;
    virtual RRHandle load_mesh(Mesh* m) = 0;
    virtual void unload_resource(RRHandle handle) = 0;
    virtual void set_render_target(RenderTarget* rt) = 0;
    virtual void set_render_targets(RenderTarget** rt, unsigned num) = 0;
    virtual void draw(const RenderObject& object, const Matrix4x4& view_matrix, const Matrix4x4& projection_matrix) = 0;
    virtual void clear_depth_stencil() = 0;
    virtual void clear_render_target(RenderTarget* sc, const Color& color) = 0;
    virtual void present() = 0;
    virtual MappedTexture map_texture(const RenderTarget& rt) = 0;
    virtual void unmap_texture(const MappedTexture& m) = 0;
    virtual void pre_draw_frame() = 0;
    virtual void set_scissor_rect(const Rect& r) = 0;
    virtual void disable_scissor() = 0;
    virtual void draw_world(const RenderWorld& world, const Quaternion& cam_rot, const Vector3& cam_pos, DrawLights draw_lights) = 0;
    virtual RRHandle load_texture(void* data, PixelFormat pf, unsigned width, unsigned height) = 0;
    virtual RenderResource& get_resource(RRHandle r) = 0;
};