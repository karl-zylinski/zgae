#include "renderer_direct3d.h"
#include <d3d11.h>
#include <D3Dcompiler.h>
#include "vertex.h"
#include "render_world.h"
#include "render_object.h"
#include "config.h"
#include "rect.h"
#include "file.h"
#include "memory.h"
#include "mesh.h"
#include "array.h"

struct RenderTargetResource
{
    ID3D11Texture2D* texture;
    ID3D11RenderTargetView* view;
};

struct ConstantBuffer
{
    Matrix4x4 model_view_projection;
    Matrix4x4 model;
    Matrix4x4 projection;
};

struct Geometry {
    ID3D11Buffer* vertices;
    ID3D11Buffer* indices;
    unsigned num_indices;
};

struct Texture
{
    ID3D11Texture2D* resource;
    ID3D11ShaderResourceView* view;
};

struct Shader
{
    ID3D11VertexShader* vertex_shader;
    ID3D11PixelShader* pixel_shader;
    ID3D11InputLayout* input_layout;
    ID3D11SamplerState* sampler_state;
};

enum struct RenderResourceType
{
    Unused,
    Geometry,
    Texture,
    Shader,
    RenderTarget,
    MappedTexture
};

struct RenderResource
{
    RenderResourceType type;
    union 
    {
        Geometry geometry;
        Texture texture;
        Shader shader;
        RenderTargetResource render_target;
    };
};

static DXGI_FORMAT pixel_format_to_dxgi_format(PixelFormat pf)
{
    switch(pf)
    {
    case PixelFormat::R8G8B8A8_UINT:
        return DXGI_FORMAT_R8G8B8A8_UINT;
    case PixelFormat::R8G8B8A8_UINT_NORM:
        return DXGI_FORMAT_R8G8B8A8_UNORM;
    case PixelFormat::R32_FLOAT:
        return DXGI_FORMAT_R32_FLOAT;
    case PixelFormat::R32G32B32A32_FLOAT:
        return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case PixelFormat::R32_UINT:
        return DXGI_FORMAT_R32_UINT;
    case PixelFormat::R8_UINT_NORM:
        return DXGI_FORMAT_R8_UNORM;
    default:
        Error("Pixel format conversion in dxgi missing.");
        return DXGI_FORMAT_UNKNOWN;
    }
}

static void check_ok(HRESULT res, ID3DBlob* error_blob)
{
    if (res == S_OK)
        return;

    static char msg[256];

    if (error_blob)
        wsprintf(msg, "Error in renderer:\n %s", (char*)error_blob->GetBufferPointer());
    else
        wsprintf(msg, "Error in renderer: %0x", res);

    MessageBox(nullptr, msg, nullptr, 0);
}

void RendererD3D::init(void* wh)
{
    window_handle = wh;
    resources = (RenderResource*)zalloc_zero(max_resources * sizeof(RenderResource));
    DXGI_SWAP_CHAIN_DESC scd = {};
    RECT window_rect = {};
    GetWindowRect((HWND)window_handle, &window_rect);
    const int w = window_rect.right - window_rect.left;
    const int h = window_rect.bottom - window_rect.top;
    scd.BufferCount = 1;
    scd.BufferDesc.Width = w;
    scd.BufferDesc.Height = h;
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = (HWND)window_handle;
    scd.SampleDesc.Count = 1;
    scd.Windowed = true;

    check_ok(D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        0,
        0,
        D3D11_SDK_VERSION,
        &scd,
        &swap_chain,
        &device,
        nullptr,
        &device_context
    ), nullptr);

    D3D11_BUFFER_DESC cbd = {};
    cbd.ByteWidth = sizeof(ConstantBuffer);
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.MiscFlags = 0;
    cbd.StructureByteStride = 0;
    device->CreateBuffer(&cbd, nullptr, &constant_buffer);
    D3D11_TEXTURE2D_DESC dstd = {};
    dstd.Width = w;
    dstd.Height = h;
    dstd.MipLevels = 1;
    dstd.ArraySize = 1;
    dstd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dstd.SampleDesc.Count = 1;
    dstd.Usage = D3D11_USAGE_DEFAULT;
    dstd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    dstd.CPUAccessFlags = 0;
    dstd.MiscFlags = 0;
    device->CreateTexture2D(&dstd, nullptr, &depth_stencil_texture);
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
    dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvd.Texture2D.MipSlice = 0;
    dsvd.Flags = 0;
    device->CreateDepthStencilView(depth_stencil_texture, &dsvd, &depth_stencil_view);

    back_buffer = create_back_buffer();
    set_render_target(&back_buffer);

    D3D11_RASTERIZER_DESC rsd;
    rsd.FillMode = D3D11_FILL_SOLID;
    rsd.CullMode = D3D11_CULL_BACK;
    rsd.FrontCounterClockwise = false;
    rsd.DepthBias = false;
    rsd.DepthBiasClamp = 0;
    rsd.SlopeScaledDepthBias = 0;
    rsd.DepthClipEnable = true;
    rsd.ScissorEnable = true;
    rsd.MultisampleEnable = false;
    rsd.AntialiasedLineEnable = false;
    device->CreateRasterizerState(&rsd, &raster_state);
    device_context->RSSetState(raster_state);
    
    disable_scissor();
}

void RendererD3D::shutdown()
{
    for (unsigned i = 0; i < max_resources; ++ i)
    {
        if (resources[i].type != RenderResourceType::Unused)
            unload_resource({i});
    }

    depth_stencil_texture->Release();
    depth_stencil_view->Release();
    device->Release();
    device_context->Release();
    zfree(resources);
    array_destroy(objects_to_render);
}

RRHandle RendererD3D::load_shader(const char* filename)
{
    unsigned handle = find_free_resource_handle();

    if (handle == InvalidHandle)
        return {InvalidHandle};

    ID3DBlob* vs_blob = nullptr;
    ID3DBlob* ps_blob = nullptr;
    ID3DBlob* error_blob = nullptr;

    LoadedFile shader_file = file_load(filename);

    if (!shader_file.valid)
        return {InvalidHandle};

    check_ok(D3DCompile(
        shader_file.file.data,
        shader_file.file.size,
        nullptr,
        nullptr,
        nullptr,
        "VShader",
        "vs_4_0",
        0,
        0,
        &vs_blob,
        &error_blob
    ), error_blob);

    check_ok(D3DCompile(
        shader_file.file.data,
        shader_file.file.size,
        nullptr,
        nullptr,
        nullptr,
        "PShader",
        "ps_4_0",
        0,
        0,
        &ps_blob,
        &error_blob
    ), error_blob);

    zfree(shader_file.file.data);

    Shader s = {};
    device->CreateVertexShader(vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), nullptr, &s.vertex_shader);
    device->CreatePixelShader(ps_blob->GetBufferPointer(), ps_blob->GetBufferSize(), nullptr, &s.pixel_shader);
    D3D11_INPUT_ELEMENT_DESC ied[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };

    device->CreateInputLayout(ied, 4, vs_blob->GetBufferPointer(), vs_blob->GetBufferSize(), &s.input_layout);
    vs_blob->Release();
    ps_blob->Release();

    D3D11_SAMPLER_DESC sd = CD3D11_SAMPLER_DESC(CD3D11_DEFAULT());
    sd.MaxAnisotropy = 0;
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    ID3D11SamplerState* ss;

    if (device->CreateSamplerState(&sd, &ss) != S_OK)
    {
        s.vertex_shader->Release();
        s.pixel_shader->Release();
        return {InvalidHandle};
    }

    s.sampler_state = ss;

    RenderResource r;
    r.type = RenderResourceType::Shader;
    r.shader = s;
    resources[handle] = r;
    return {handle};
}

void RendererD3D::set_shader(RRHandle shader)
{
    Shader& s = get_resource(shader).shader;
    device_context->VSSetShader(s.vertex_shader, 0, 0);
    device_context->PSSetShader(s.pixel_shader, 0, 0);
    device_context->IASetInputLayout(s.input_layout);

    if (s.sampler_state != nullptr)
    {
        device_context->PSSetSamplers(0, 1, &s.sampler_state);
    }
}

RenderTarget RendererD3D::create_back_buffer()
{
    unsigned handle = find_free_resource_handle();
    Assert(handle != InvalidHandle, "Couldn't create back-buffer.");

    RenderTargetResource rts = {};
    ID3D11Texture2D* back_buffer_texture;
    swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer_texture);

    D3D11_TEXTURE2D_DESC td = {};
    back_buffer_texture->GetDesc(&td);

    device->CreateRenderTargetView(back_buffer_texture, nullptr, &rts.view);
    back_buffer_texture->Release();

    RenderResource r = {};
    r.type = RenderResourceType::RenderTarget;
    r.render_target = rts;
    resources[handle] = r;

    RenderTarget rt;
    rt.render_resource = {handle};
    rt.width = td.Width;
    rt.height = td.Height;
    rt.clear = true;
    rt.clear_depth_stencil = true;
    rt.clear_color = {0, 0, 0, 1};
    return rt;
}

RenderTarget RendererD3D::create_render_texture(PixelFormat pf, unsigned width, unsigned height)
{
    unsigned handle = find_free_resource_handle();
    Assert(handle != InvalidHandle, "Couldn't create render texture.");

    D3D11_TEXTURE2D_DESC rtd = {};
    rtd.Width = width;
    rtd.Height = height;
    rtd.MipLevels = 1;
    rtd.ArraySize = 1;
    rtd.Format = pixel_format_to_dxgi_format(pf);
    rtd.SampleDesc.Count = 1;
    rtd.Usage = D3D11_USAGE_DEFAULT;
    rtd.BindFlags = D3D11_BIND_RENDER_TARGET;
    rtd.CPUAccessFlags = 0;
    rtd.MiscFlags = 0;
    ID3D11Texture2D* texture;
    device->CreateTexture2D(&rtd, NULL, &texture);
    D3D11_RENDER_TARGET_VIEW_DESC rtvd = {};
    rtvd.Format = rtd.Format;
    rtvd.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvd.Texture2D.MipSlice = 0;

    RenderTargetResource rts = {};
    rts.texture = texture;
    device->CreateRenderTargetView(texture, &rtvd, &rts.view);
    RenderResource r = {};
    r.type = RenderResourceType::RenderTarget;
    r.render_target = rts;
    resources[handle] = r;

    RenderTarget rt = {};
    rt.render_resource = {handle};
    rt.pixel_format = pf;
    rt.width = width;
    rt.height = height;
    rt.clear = true;
    rt.clear_depth_stencil = true;
    rt.clear_color = {0, 0, 0, 1};
    return rt;
}

static void set_constant_buffers(ID3D11DeviceContext* device_context, ID3D11Buffer* constant_buffer, const ConstantBuffer& data)
{
    D3D11_MAPPED_SUBRESOURCE ms_constant_buffer;
    device_context->Map(constant_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ms_constant_buffer);
    memcpy(ms_constant_buffer.pData, &data, sizeof(ConstantBuffer));
    device_context->Unmap(constant_buffer, 0);
}

unsigned RendererD3D::find_free_resource_handle() const
{
    for (unsigned i = 1; i < max_resources; ++i)
    {
        if (resources[i].type == RenderResourceType::Unused)
        {
            return i;
        }
    }

    return InvalidHandle;
}

RRHandle RendererD3D::load_mesh(Mesh* m)
{
    unsigned handle = find_free_resource_handle();

    if (handle == InvalidHandle)
        return {InvalidHandle};

    ID3D11Buffer* vertex_buffer;
    {
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = sizeof(Vertex) * m->num_vertices;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        D3D11_SUBRESOURCE_DATA srd = {};
        srd.pSysMem = m->vertices;
        device->CreateBuffer(&bd, &srd, &vertex_buffer);
    }

    ID3D11Buffer* index_buffer;
    {
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = sizeof(unsigned) * m->num_indices;
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        D3D11_SUBRESOURCE_DATA srd = {0};
        srd.pSysMem = m->indices;
        device->CreateBuffer(&bd, &srd, &index_buffer);
    }

    Geometry g = {};
    g.vertices = vertex_buffer;
    g.indices = index_buffer;
    g.num_indices = m->num_indices;

    RenderResource r;
    r.type = RenderResourceType::Geometry;
    r.geometry = g;
    resources[handle] = r;
    return {handle};
}

void RendererD3D::unload_resource(RRHandle handle)
{
    RenderResource& res = get_resource(handle);

    switch (res.type)
    {
        case RenderResourceType::Geometry:
            res.geometry.vertices->Release();
            res.geometry.indices->Release();
            break;

        case RenderResourceType::Texture:
            res.texture.view->Release();
            res.texture.resource->Release();
            break;

        case RenderResourceType::MappedTexture:
            res.texture.resource->Release();
            break;

        case RenderResourceType::Shader:
            res.shader.input_layout->Release();
            res.shader.vertex_shader->Release();
            res.shader.pixel_shader->Release();
            break;

        case RenderResourceType::RenderTarget:
            res.render_target.view->Release();

            if (res.render_target.texture != nullptr)
                res.render_target.texture->Release();
            
            break;

        default:
            if (res.type != RenderResourceType::Unused)
            {
                Error("Missing resource unloader.");
            }
            break;
    }

    memset(&res, 0, sizeof(RenderResource));
}

void RendererD3D::set_render_target(RenderTarget* rt)
{
    set_render_targets(&rt, 1);
}

void RendererD3D::set_render_targets(RenderTarget** rts, unsigned num)
{
    Assert(num > 0, "Trying to set 0 render targets.");
    ID3D11RenderTargetView* targets[4];
    for (unsigned i = 0; i < num; ++i)
    {
        targets[i] = get_resource(rts[i]->render_resource).render_target.view;
    }
    device_context->OMSetRenderTargets(num, targets, depth_stencil_view);
    memset(render_targets, 0, sizeof(render_targets));
    memcpy(render_targets, rts, sizeof(RenderTarget**) * num);

    D3D11_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (float)rts[0]->width;
    viewport.Height = (float)rts[0]->height;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;
    device_context->RSSetViewports(1, &viewport);
}

void RendererD3D::draw(const RenderObject& object, const Matrix4x4& view_matrix, const Matrix4x4& projection_matrix)
{
    auto geometry = get_resource(object.geometry_handle).geometry;
    ConstantBuffer constant_buffer_data = {};
    constant_buffer_data.model_view_projection = object.world_transform * view_matrix * projection_matrix;
    constant_buffer_data.model = object.world_transform;
    constant_buffer_data.projection = projection_matrix;
    set_constant_buffers(device_context, constant_buffer, constant_buffer_data);
    device_context->VSSetConstantBuffers(0, 1, &constant_buffer);
    device_context->PSSetConstantBuffers(0, 1, &constant_buffer);

    unsigned stride = sizeof(Vertex);
    unsigned offset = 0;
    device_context->IASetVertexBuffers(0, 1, &geometry.vertices, &stride, &offset);
    device_context->IASetIndexBuffer(geometry.indices, DXGI_FORMAT_R32_UINT, 0);
    device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    device_context->DrawIndexed(geometry.num_indices, 0, 0);
}

void RendererD3D::clear_depth_stencil()
{
    device_context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void RendererD3D::clear_render_target(RenderTarget* sc, const Color& color)
{
    device_context->ClearRenderTargetView(get_resource(sc->render_resource).render_target.view, &color.r);
}

void RendererD3D::present()
{
    swap_chain->Present(0, 0);
}

MappedTexture RendererD3D::map_texture(const RenderTarget& rt)
{
    unsigned handle = find_free_resource_handle();
    Assert(handle != InvalidHandle, "Out of handles.");

    D3D11_TEXTURE2D_DESC rtd = {};
    ID3D11Texture2D* texture = get_resource(rt.render_resource).render_target.texture;
    texture->GetDesc(&rtd);
    rtd.Usage = D3D11_USAGE_STAGING;
    rtd.BindFlags = 0;
    rtd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    ID3D11Texture2D* staging_texture;
    device->CreateTexture2D(&rtd, NULL, &staging_texture);
    device_context->CopyResource(staging_texture, texture);
    D3D11_MAPPED_SUBRESOURCE mapped_resource;
    device_context->Map(staging_texture, 0, D3D11_MAP_READ, 0, &mapped_resource);

    Texture t = {};
    t.resource = staging_texture;

    RenderResource r = {};
    r.type = RenderResourceType::MappedTexture;
    r.texture = t;
    resources[handle] = r;

    MappedTexture m = {};
    m.data = mapped_resource.pData;
    m.texture = {handle};
    return m;
}

void RendererD3D::unmap_texture(const MappedTexture& m)
{
    ID3D11Texture2D* tex = get_resource(m.texture).texture.resource;
    device_context->Unmap(tex, 0);
    unload_resource(m.texture);
}

void RendererD3D::pre_draw_frame()
{
    bool clear_depth = false;
    for (unsigned i = 0; i < max_render_targets; ++i)
    {
        RenderTarget* r = render_targets[i];

        if (r == nullptr)
        {
            continue;
        }

        if (r->clear)
        {
            clear_render_target(r, r->clear_color);
        }

        if (r->clear_depth_stencil)
        {
            clear_depth = true;
        }
    }

    if (clear_depth)
    {
        clear_depth_stencil();
    }
}

void RendererD3D::draw_world(const RenderWorld& world, const Quaternion& cam_rot, const Vector3& cam_pos, const Matrix4x4& projection, DrawLights draw_lights)
{
    pre_draw_frame();
    Matrix4x4 view_matrix = matrix4x4_inverse(matrix4x4_from_rotation_and_translation(cam_rot, cam_pos));

    array_empty(objects_to_render);
    render_world_get_objects_to_render(&world, &objects_to_render);

    for (unsigned i = 0; i < array_num(objects_to_render); ++i)
        draw(objects_to_render[i], view_matrix, projection);

    present();
}

void RendererD3D::set_scissor_rect(const Rect& r)
{
    static D3D11_RECT rect = {};
    rect.top = r.top;
    rect.bottom = r.bottom;
    rect.left = r.left;
    rect.right = r.right;
    device_context->RSSetScissorRects(1, &rect);
}

void RendererD3D::disable_scissor()
{
    RECT window_rect = {};
    GetWindowRect((HWND)window_handle, &window_rect);
    const int w = window_rect.right - window_rect.left;
    const int h = window_rect.bottom - window_rect.top;

    D3D11_RECT rect = {};
    rect.top = 0;
    rect.bottom = h;
    rect.left = 0;
    rect.right = w;
    device_context->RSSetScissorRects(1, &rect);
}

RRHandle RendererD3D::load_texture(void* data, PixelFormat pf, unsigned width, unsigned height)
{
    unsigned handle = find_free_resource_handle();

    if (handle == InvalidHandle)
        return {InvalidHandle};

    D3D11_TEXTURE2D_DESC desc;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = pixel_format_to_dxgi_format(pf);
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA init_data;
    init_data.pSysMem = data;
    init_data.SysMemPitch = width * pixel_size(pf);
    init_data.SysMemSlicePitch = image_size(pf, width, height);

    ID3D11Texture2D* tex;
    if (device->CreateTexture2D(&desc, &init_data, &tex) != S_OK)
        return {InvalidHandle};

    ID3D11ShaderResourceView* resource_view;

    if (device->CreateShaderResourceView(tex, nullptr, &resource_view) != S_OK)
    {
        tex->Release();
        return {InvalidHandle};
    }

    RenderResource r;
    r.type = RenderResourceType::Texture;
    r.texture.resource = tex;
    r.texture.view = resource_view;
    resources[handle] = r;
    return {handle};
}

RenderResource& RendererD3D::get_resource(RRHandle r)
{
    Assert(r.h > 0 && r.h < max_resources, "Resource handle out of bounds.");
    return resources[r.h];
}
