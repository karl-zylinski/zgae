#include "renderer.h"
#include "window_types.h"
#include "renderer_backend.h"
#include "memory.h"
#include "handle_pool.h"
#include "array.h"
#include "debug.h"
#include "pipeline.h"
#include "shader.h"
#include "str.h"

fwd_struct(RendererBackendState);
fwd_struct(RendererBackendShader);
fwd_struct(RendererBackendPipeline);
fwd_struct(RendererBackendGeometry);

typedef struct RendererResourceShader
{
    char* source;
    u64 source_size;
    ShaderType type;
    ShaderConstantBuffer constant_buffer;
    ShaderInputLayoutItem* input_layout;
    u32 input_layout_num;
    RendererBackendShader* backend_state;
} RendererResourceShader;

typedef struct RendererResourcePipeline
{
    RendererResourceHandle* shader_stages;
    u32 shader_stages_num;
    RendererBackendPipeline* backend_state;
} RendererResourcePipeline;

typedef struct RendererResourceGeometry
{
    GeometryVertex* vertices;
    GeometryIndex* indices;
    u32 vertices_num;
    u32 indices_num;
    RendererBackendGeometry* backend_state;
} RendererResourceGeometry;

typedef enum RendererResourceType
{
    RENDERER_RESOURCE_TYPE_INVALID,
    RENDERER_RESOURCE_TYPE_SHADER,
    RENDERER_RESOURCE_TYPE_PIPELINE,
    RENDERER_RESOURCE_TYPE_GEOMETRY,
    RENDERER_RESOURCE_TYPE_NUM
} RendererResourceType;

static const char* renderer_resouce_type_names[] =
{
    "invalid", "shader", "pipeline", "geometry"
};

typedef enum RendererResourceFlag
{
    RENDERER_RESOURCE_FLAG_SURFACE_SIZE_DEPENDENT = 0x1
} RendererResourceFlag;

typedef struct RendererResource
{
    RendererResourceType type;
    RendererResourceHandle handle;
    RendererResourceFlag flag;

    union
    {
        RendererResourceShader shader;
        RendererResourcePipeline pipeline;
        RendererResourceGeometry geometry;
    };
} RendererResource;

typedef struct RendererState
{
    RendererBackendState* rbs;
    RendererResource* da_resources;
    HandlePool* resource_handle_pool;
} RendererState;

RendererState* renderer_create(WindowType window_type, void* window_data)
{
    RendererState* rs = mema_zero(sizeof(RendererState));
    rs->resource_handle_pool = handle_pool_create();

    for (RendererResourceType t = 1; t < RENDERER_RESOURCE_TYPE_NUM; ++t)
        handle_pool_set_type(rs->resource_handle_pool, t, renderer_resouce_type_names[t]);
    
    RendererBackendState* rbs = renderer_backend_create(window_type, window_data);
    rs->rbs = rbs;
    return rs;
}

static RendererResource* get_resource(RendererState* rs, RendererResourceHandle h)
{
    RendererResource* rr = rs->da_resources + handle_index(h);
    check_slow(handle_type(h) == rr->type, "Handle points to resource of wrong type");
    return rr;
}

static void deinit_resource(RendererState* rs, RendererResource* rr)
{
    switch(rr->type)
    {
        case RENDERER_RESOURCE_TYPE_SHADER: {
            renderer_backend_destroy_shader(rs->rbs, rr->shader.backend_state);
        } break;
        
        case RENDERER_RESOURCE_TYPE_PIPELINE: {
            renderer_backend_destroy_pipeline(rs->rbs, rr->pipeline.backend_state);
        } break;

        case RENDERER_RESOURCE_TYPE_GEOMETRY: {
            renderer_backend_destroy_geometry(rs->rbs, rr->geometry.backend_state);
        } break;

        case RENDERER_RESOURCE_TYPE_NUM:
        case RENDERER_RESOURCE_TYPE_INVALID:
            error("Invalid resource in renderer resource list"); break;
    }
}

static RendererBackendShader* shader_init(RendererState* rs, const RendererResourceShader* shader)
{
    return renderer_backend_create_shader(rs->rbs, shader->source, shader->source_size);
}

static RendererBackendPipeline* pipeline_init(RendererState* rs, const RendererResourcePipeline* pipeline)
{
    RendererBackendShader** backend_shader_stages = mema(sizeof(RendererBackendShader*) * pipeline->shader_stages_num);
    ShaderType* backend_shader_types = mema(sizeof(ShaderType) * pipeline->shader_stages_num);
    ShaderDataType* vertex_input_types = NULL;
    u32 vertex_input_types_num = 0;
    u32* da_constant_buffer_sizes = NULL;
    u32* da_constant_buffer_binding_indices = NULL;

    for (u32 shdr_idx = 0; shdr_idx < pipeline->shader_stages_num; ++shdr_idx)
    {
        RendererResourceShader* shader = &get_resource(rs, pipeline->shader_stages[shdr_idx])->shader;
        backend_shader_stages[shdr_idx] = shader->backend_state;
        backend_shader_types[shdr_idx] = shader->type;

        if (shader->input_layout_num > 0)
        {
            check(!vertex_input_types, "Pipeline contains multiple shader that specify vertex shader inputs");
            vertex_input_types_num = shader->input_layout_num;
            vertex_input_types = mema(sizeof(ShaderDataType) * vertex_input_types_num);

            for (u32 ipt_idx = 0; ipt_idx < vertex_input_types_num; ++ipt_idx)
                vertex_input_types[ipt_idx] = shader->input_layout[ipt_idx].type;
        }

        if (shader->constant_buffer.items_num > 0)
        {
            u32 s = 0;

            for (u32 cb_item_idx = 0; cb_item_idx < shader->constant_buffer.items_num; ++cb_item_idx)
                s += shader_data_type_size(shader->constant_buffer.items[cb_item_idx].type);

            array_add(da_constant_buffer_sizes, s);
            array_add(da_constant_buffer_binding_indices, shader->constant_buffer.binding);
        }
    }

    u32 constant_buffers_num = array_num(da_constant_buffer_sizes);

    check(vertex_input_types, "Pipeline has no shader that specifies vertex inputs");

    RendererBackendPipeline* backend_state = renderer_backend_create_pipeline(
        rs->rbs, backend_shader_stages, backend_shader_types, pipeline->shader_stages_num,
        vertex_input_types, vertex_input_types_num,
        da_constant_buffer_sizes, da_constant_buffer_binding_indices, constant_buffers_num);

    array_destroy(da_constant_buffer_binding_indices);
    array_destroy(da_constant_buffer_sizes);
    memf(vertex_input_types);
    memf(backend_shader_types);
    memf(backend_shader_stages);

    return backend_state;
}

static RendererBackendGeometry* geometry_init(RendererState* rs, const RendererResourceGeometry* g)
{
    return renderer_backend_create_geometry(rs->rbs, g->vertices, g->vertices_num, g->indices, g->indices_num);
}

static void init_resource(RendererState* rs, RendererResource* rr)
{
    switch(rr->type)
    {
        case RENDERER_RESOURCE_TYPE_SHADER: {
            RendererResourceShader* s = &rr->shader;
            s->backend_state = shader_init(rs, s);
        } break;
        
        case RENDERER_RESOURCE_TYPE_PIPELINE: {
            RendererResourcePipeline* p = &rr->pipeline;
            p->backend_state = pipeline_init(rs, p);
        } break;

        case RENDERER_RESOURCE_TYPE_GEOMETRY: {
            RendererResourceGeometry* g = &rr->geometry;
            g->backend_state = geometry_init(rs, g);
        } break;

        case RENDERER_RESOURCE_TYPE_NUM:
        case RENDERER_RESOURCE_TYPE_INVALID:
            error("Invalid resource in renderer resource list"); break;
    }
}

static void destroy_resource(RendererState* rs, RendererResource* rr)
{
    deinit_resource(rs, rr);

    switch(rr->type)
    {
        case RENDERER_RESOURCE_TYPE_SHADER: {
            RendererResourceShader* s = &rr->shader;
            memf(s->source);
            memf(s->constant_buffer.items);

            for (u32 i = 0; i < s->input_layout_num; ++i)
                memf(s->input_layout[i].name);

            memf(s->input_layout);
        } break;
        
        case RENDERER_RESOURCE_TYPE_PIPELINE: {
            RendererResourcePipeline* p = &rr->pipeline;
            memf(p->shader_stages);
        } break;

        case RENDERER_RESOURCE_TYPE_GEOMETRY: {
            RendererResourceGeometry* g = &rr->geometry;
            memf(g->vertices);
            memf(g->indices);
        } break;

        case RENDERER_RESOURCE_TYPE_NUM:
        case RENDERER_RESOURCE_TYPE_INVALID:
            error("Invalid resource in renderer resource list"); break;
    }

    handle_pool_return(rs->resource_handle_pool, rr->handle);
}

static void reinit_resource(RendererState* rs, RendererResource* rr)
{
    deinit_resource(rs, rr);
    init_resource(rs, rr);
}

static void destroy_all_resources(RendererState* rs)
{
    info("Destroying all renderer resources");

    for (sizet i = 0; i < array_num(rs->da_resources); ++i)
        destroy_resource(rs, rs->da_resources + i);
}

void renderer_destroy(RendererState* rs)
{
    info("Destroying renderer");
    renderer_backend_wait_until_idle(rs->rbs);
    destroy_all_resources(rs);
    array_destroy(rs->da_resources);
    renderer_backend_destroy(rs->rbs);
    handle_pool_destroy(rs->resource_handle_pool);
    memf(rs);
}

static RendererResourceHandle add_resource(HandlePool* hp, RendererResource** da_resources, RendererResource* res)
{
    RendererResourceHandle h = handle_pool_reserve(hp, res->type);
    res->handle = h;
    array_fill_and_set(*da_resources, handle_index(h), *res);
    return h;
}

RendererResourceHandle renderer_load_shader(RendererState* rs, const ShaderIntermediate* si)
{
    RendererResource shader_res = {};
    shader_res.type = RENDERER_RESOURCE_TYPE_SHADER;
    RendererResourceShader* shader = &shader_res.shader;
    shader->source = mema_copy(si->source, si->source_size);
    shader->source_size = si->source_size;
    shader->type = si->type;
    shader->constant_buffer.items = mema_copy(si->constant_buffer.items, si->constant_buffer.items_num * sizeof(ShaderConstantBufferItem));
    shader->constant_buffer.items_num = si->constant_buffer.items_num;
    shader->constant_buffer.binding = si->constant_buffer.binding;
    shader->input_layout = mema_copy(si->input_layout, si->input_layout_num * sizeof(ShaderInputLayoutItem));
    shader->input_layout_num = si->input_layout_num;

    for (u32 i = 0; i < shader->input_layout_num; ++i)
        shader->input_layout[i].name = str_copy(shader->input_layout[i].name);

    init_resource(rs, &shader_res);

    return add_resource(rs->resource_handle_pool, &rs->da_resources, &shader_res);
}

RendererResourceHandle renderer_load_pipeline(RendererState* rs, const PipelineIntermediate* pi)
{
    RendererResource pipeline_res = {};
    pipeline_res.type = RENDERER_RESOURCE_TYPE_PIPELINE;
    RendererResourcePipeline* pipeline = &pipeline_res.pipeline;
    pipeline->shader_stages = mema_copy(pi->shader_stages, sizeof(RendererResourceHandle) * pi->shader_stages_num);
    pipeline->shader_stages_num = pi->shader_stages_num;
    pipeline_res.flag = RENDERER_RESOURCE_FLAG_SURFACE_SIZE_DEPENDENT; // TODO, ask backend about this?

    init_resource(rs, &pipeline_res);

    return add_resource(rs->resource_handle_pool, &rs->da_resources, &pipeline_res);
}

RendererResourceHandle renderer_load_geometry(RendererState* rs, const GeometryVertex* vertices, u32 vertices_num, const GeometryIndex* indices, u32 indices_num)
{
    RendererResourceGeometry g = {
        .vertices = mema_copy(vertices, sizeof(GeometryVertex) * vertices_num),
        .vertices_num = vertices_num,
        .indices = mema_copy(indices, sizeof(GeometryIndex) * indices_num),
        .indices_num = indices_num
    };

    RendererResource geometry_res = {
        .type = RENDERER_RESOURCE_TYPE_GEOMETRY,
        .geometry = g
    };

    init_resource(rs, &geometry_res);
    return add_resource(rs->resource_handle_pool, &rs->da_resources, &geometry_res);
}

void renderer_draw(RendererState* rs, RendererResourceHandle pipeline_handle, RendererResourceHandle geometry_handle)
{
    renderer_backend_draw(rs->rbs, get_resource(rs, pipeline_handle)->pipeline.backend_state, get_resource(rs, geometry_handle)->geometry.backend_state);
}

void renderer_present(RendererState* rs)
{
    renderer_backend_present(rs->rbs);
}

void renderer_update_constant_buffer(RendererState* rs, RendererResourceHandle pipeline_handle, u32 binding, void* data, u32 data_size)
{
    renderer_backend_update_constant_buffer(rs->rbs, get_resource(rs, pipeline_handle)->pipeline.backend_state, binding, data, data_size);
}

void renderer_wait_for_new_frame(RendererState* rs)
{
    renderer_backend_wait_for_new_frame(rs->rbs);
}

void renderer_surface_resized(RendererState* rs, u32 w, u32 h)
{
    info("Renderer resizing to %d x %d", w, h);
    renderer_backend_wait_until_idle(rs->rbs);
    renderer_backend_surface_resized(rs->rbs, w, h);

    for (sizet i = 0; i < array_num(rs->da_resources); ++i)
    {
        RendererResource* rr = rs->da_resources + i;

        if (rr->flag & RENDERER_RESOURCE_FLAG_SURFACE_SIZE_DEPENDENT)
            reinit_resource(rs, rr);
    }
    renderer_backend_wait_until_idle(rs->rbs);
}