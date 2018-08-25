#pragma once

#include "image.h"
#include "color.h"
#include "math.h"
#include "render_resource.h"
#define IsValidRRHandle(rrh) rrh.h != InvalidHandle

struct ID3D11Buffer;
struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
struct ID3D11Texture2D;
struct ID3D11DepthStencilView;
struct ID3D11RasterizerState;
struct IDXGISwapChain;
struct World;
struct Object;
struct Camera;
struct Vertex;
struct Rect;
struct RenderResource;

Image image_from_render_target(const RenderTarget& rt);

enum struct DrawLights { DrawLights, DoNotDrawLights };

struct Renderer
{
    void init(void* window_handle);
    void shutdown();
    RRHandle load_shader(const char* filename);
    void set_shader(RRHandle shader);
    RenderTarget create_back_buffer();
    RenderTarget create_render_texture(PixelFormat pf, unsigned width, unsigned height);
    unsigned find_free_resource_handle() const;
    RRHandle load_geometry(Vertex* vertices, unsigned num_vertices, unsigned* indices, unsigned num_indices);
    void unload_resource(RRHandle handle);
    void set_render_target(RenderTarget* rt);
    void set_render_targets(RenderTarget** rt, unsigned num);
    void draw(const Object& object, const Matrix4x4& view_matrix, const Matrix4x4& projection_matrix);
    void clear_depth_stencil();
    void clear_render_target(RenderTarget* sc, const Color& color);
    void present();
    MappedTexture map_texture(const RenderTarget& rt);
    void unmap_texture(const MappedTexture& m);
    void pre_draw_frame();
    void set_scissor_rect(const Rect& r);
    void disable_scissor();
    void draw_frame(const World& world, const Camera& camera, DrawLights draw_lights);
    RRHandle load_texture(void* data, PixelFormat pf, unsigned width, unsigned height);
    RenderResource& get_resource(RRHandle r);

    static const unsigned max_resources = 4096;
    static const unsigned max_render_targets = 4;
    void* window_handle;
    ID3D11Device* device;
    ID3D11DeviceContext* device_context;
    ID3D11Buffer* constant_buffer;
    ID3D11Texture2D* depth_stencil_texture;
    ID3D11DepthStencilView* depth_stencil_view;
    ID3D11RasterizerState* raster_state;
    IDXGISwapChain* swap_chain;
    RenderTarget back_buffer;
    RenderResource* resources;
    RenderTarget* render_targets[max_render_targets];
};
