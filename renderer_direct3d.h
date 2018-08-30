#pragma once

#include "renderer.h"
#include "image.h"
#include "color.h"
#include "math.h"
#define IsValidRRHandle(rrh) rrh.h != InvalidHandle

struct ID3D11Buffer;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;
struct ID3D11RasterizerState;
struct IDXGISwapChain;
struct Object;
struct RenderObject;

struct RendererD3D : public Renderer
{
    void init(void* window_handle);
    void shutdown();
    RRHandle load_shader(const ShaderIntermediate& si);
    void set_shader(RRHandle shader);
    void set_constant_buffer_data(RRHandle shader, const char* name, void* data, unsigned data_size);
    RenderTarget create_render_texture(PixelFormat pf, unsigned width, unsigned height);
    RRHandle load_mesh(Mesh* m);
    void unload_resource(RRHandle handle);
    void set_render_target(RenderTarget* rt);
    void set_render_targets(RenderTarget** rt, unsigned num);
    void present();
    MappedTexture map_texture(const RenderTarget& rt);
    void unmap_texture(const MappedTexture& m);
    void pre_frame();
    void set_scissor_rect(const Rect& r);
    void disable_scissor();
    void draw_world(const RenderWorld& world, const Quat& cam_rot, const Vec3& cam_pos);
    RRHandle load_texture(void* data, PixelFormat pf, unsigned width, unsigned height);
    RenderResource& get_resource(RRHandle r);
    void draw_debug_mesh(const Vec3* vertices, unsigned num_vertices, const Color& color);
    void resize_back_buffer(unsigned width, unsigned height);

private:
    void load_constant_buffers(const Mat4& world_transform, const Mat4& view_matrix, const Mat4& projection_matrix);
    void create_back_buffer();
    void create_depth_stencil_view(unsigned width, unsigned height);
    unsigned find_free_resource_handle() const;
    void draw(const RenderObject& object, const Mat4& view_matrix, const Mat4& projection_matrix);
    void clear_depth_stencil();
    void clear_render_target(RenderTarget* sc, const Color& color);
    void create_projection_matrix();
    
    static const unsigned max_resources = 4096;
    static const unsigned max_render_targets = 4;
    void* _window_handle;
    ID3D11Device* _device;
    ID3D11DeviceContext* _device_context;
    ID3D11Texture2D* _depth_stencil_texture;
    ID3D11DepthStencilView* _depth_stencil_view;
    ID3D11RasterizerState* _raster_state;
    IDXGISwapChain* _swap_chain;
    RenderTarget _back_buffer;
    RenderResource* _resources;
    RenderTarget* _render_targets[max_render_targets];
    RenderObject* _objects_to_render;
    RRHandle _debug_shader;
    RRHandle _current_shader;
    Mat4 _projection_matrix;
    Mat4 _latest_view_matrix;
};
