#pragma once
#include "render_resource.h"

struct Rect;
struct RenderWorld;
struct RenderResource;
struct Quat;
struct Vec3;
struct Mesh;
struct ShaderIntermediate;

struct Renderer
{
    virtual void init(void* window_handle) = 0;
    virtual void shutdown() = 0;
    virtual RRHandle load_shader(const ShaderIntermediate& si) = 0;
    virtual void set_shader(RRHandle shader) = 0;
    virtual RenderTarget create_render_texture(PixelFormat pf, unsigned width, unsigned height) = 0;
    virtual RRHandle load_mesh(Mesh* m) = 0;
    virtual void unload_resource(RRHandle handle) = 0;
    virtual void set_render_target(RenderTarget* rt) = 0;
    virtual void set_render_targets(RenderTarget** rt, unsigned num) = 0;
    virtual void present() = 0;
    virtual MappedTexture map_texture(const RenderTarget& rt) = 0;
    virtual void unmap_texture(const MappedTexture& m) = 0;
    virtual void pre_frame() = 0;
    virtual void set_scissor_rect(const Rect& r) = 0;
    virtual void disable_scissor() = 0;
    virtual void draw_world(const RenderWorld& world, const Quat& cam_rot, const Vec3& cam_pos) = 0;
    virtual RRHandle load_texture(void* data, PixelFormat pf, unsigned width, unsigned height) = 0;
    virtual RenderResource& get_resource(RRHandle r) = 0;
    virtual void draw_debug_mesh(const Vec3* vertices, unsigned num_vertices) = 0;
};