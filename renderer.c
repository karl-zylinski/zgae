#include "renderer.h"
#include "window_types.h"
#include "renderer_backend.h"
#include "memory.h"
#include "handle_pool.h"
#include "array.h"
#include "debug.h"
#include "shader_resource.h"
#include "geometry_types.h"
#include "math.h"
#include "resource.h"
#include "resource_types.h"

typedef struct RendererResourceShader
{
    ResourceHandle res;
    RendererBackendShader* backend_state;
} RendererResourceShader;

typedef struct RendererResourcePipeline
{
    ResourceHandle res;
    RendererBackendPipeline* backend_state;
} RendererResourcePipeline;

typedef struct RendererResourceGeometry
{
    Mesh mesh;
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
    rs->resource_handle_pool = handle_pool_create(1, "RendererResourceHandle");

    for (RendererResourceType s = 1; s < RENDERER_RESOURCE_TYPE_NUM; ++s)
        handle_pool_set_subtype(rs->resource_handle_pool, s, renderer_resouce_type_names[s]);
    
    RendererBackendState* rbs = renderer_backend_create(window_type, window_data);
    rs->rbs = rbs;
    return rs;
}

static RendererResource* get_resource(RendererState* rs, RendererResourceHandle h)
{
    RendererResource* rr = rs->da_resources + handle_index(h);
    check_slow(handle_subtype(h) == rr->type, "Handle points to resource of wrong type");
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
    const ShaderResource* r = &resource_lookup(shader->res)->shader;
    return renderer_backend_create_shader(rs->rbs, r->source, r->source_size);
}

static RendererBackendPipeline* pipeline_init(RendererState* rs, RendererResourcePipeline* pipeline)
{
    const PipelineResource* pr = &resource_lookup(pipeline->res)->pipeline;

    RendererBackendShader** backend_shader_stages = mema(sizeof(RendererBackendShader*) * pr->shader_stages_num);
    ShaderType* backend_shader_types = mema(sizeof(ShaderType) * pr->shader_stages_num);
    for (u32 shdr_idx = 0; shdr_idx < pr->shader_stages_num; ++shdr_idx)
    {
        const Resource* shader_resource = resource_lookup(pr->shader_stages[shdr_idx]);
        RendererResourceShader* srr = &get_resource(rs, shader_resource->rrh)->shader;
        backend_shader_stages[shdr_idx] = srr->backend_state;
        backend_shader_types[shdr_idx] = shader_resource->shader.type;
    }

    ShaderDataType* vertex_input_types = mema(sizeof(ShaderDataType) * pr->vertex_input_num);
    for (u32 vi_idx = 0; vi_idx < pr->vertex_input_num; ++vi_idx)
        vertex_input_types[vi_idx] = pr->vertex_input[vi_idx].type;

    u32* constant_buffer_sizes = mema(sizeof(u32) * pr->constant_buffers_num);
    u32* constant_buffer_binding_indices = mema(sizeof(u32) * pr->constant_buffers_num);

    for (u32 cb_idx = 0; cb_idx < pr->constant_buffers_num; ++cb_idx)
    {
        u32 s = 0;

        for (u32 cb_field_idx = 0; cb_field_idx < pr->constant_buffers[cb_idx].fields_num; ++cb_field_idx)
            s += shader_data_type_size(pr->constant_buffers[cb_idx].fields[cb_field_idx].type);

        constant_buffer_sizes[cb_idx] = s;
        constant_buffer_binding_indices[cb_idx] = pr->constant_buffers[cb_idx].binding;
    }

    RendererBackendPipeline* backend_state = renderer_backend_create_pipeline(
        rs->rbs, backend_shader_stages, backend_shader_types, pr->shader_stages_num,
        vertex_input_types, pr->vertex_input_num,
        constant_buffer_sizes, constant_buffer_binding_indices, pr->constant_buffers_num);

    memf(constant_buffer_binding_indices);
    memf(constant_buffer_sizes);
    memf(vertex_input_types);
    memf(backend_shader_types);
    memf(backend_shader_stages);

    return backend_state;
}

static RendererBackendGeometry* geometry_init(RendererState* rs, const RendererResourceGeometry* g)
{
    return renderer_backend_create_geometry(rs->rbs, &g->mesh);
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
        } break;
        
        case RENDERER_RESOURCE_TYPE_PIPELINE: {
        } break;

        case RENDERER_RESOURCE_TYPE_GEOMETRY: {
            RendererResourceGeometry* g = &rr->geometry;
            memf(g->mesh.vertices);
            memf(g->mesh.indices);
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


void renderer_destroy(RendererState* rs)
{
    info("Destroying renderer");
    renderer_backend_wait_until_idle(rs->rbs);
    
    info("Destroying all renderer resources");
    for (size_t i = 0; i < array_num(rs->da_resources); ++i)
        destroy_resource(rs, rs->da_resources + i);

    array_destroy(rs->da_resources);
    renderer_backend_destroy(rs->rbs);
    handle_pool_destroy(rs->resource_handle_pool);
    memf(rs);
}

static RendererResourceHandle add_resource(HandlePool* hp, RendererResource** da_resources, RendererResource* res)
{
    RendererResourceHandle h = handle_pool_borrow(hp, res->type);
    res->handle = h;
    array_fill_and_set(*da_resources, handle_index(h), *res);
    return h;
}

RendererResourceHandle renderer_create_renderer_resource(RendererState* rs, ResourceHandle resource_handle)
{
    const Resource* r = resource_lookup(resource_handle);
    check(r, "Trying to create renderer resource for invalid resource");

    RendererResource rr = {};

    switch(r->type)
    {
        case RESOURCE_TYPE_SHADER: {
            rr.type = RENDERER_RESOURCE_TYPE_SHADER;
            rr.shader.res = resource_handle;
        } break;

        case RESOURCE_TYPE_PIPELINE: {
            rr.type = RENDERER_RESOURCE_TYPE_PIPELINE;
            rr.flag = RENDERER_RESOURCE_FLAG_SURFACE_SIZE_DEPENDENT;
            rr.pipeline.res = resource_handle;
        } break;

        default: error("Implement me!"); break;
    }

    init_resource(rs, &rr);
    return add_resource(rs->resource_handle_pool, &rs->da_resources, &rr);
}

RendererResourceHandle renderer_load_geometry(RendererState* rs, const Mesh* mesh)
{
    RendererResourceGeometry g = {
        .mesh = *mesh
    };

    g.mesh.vertices = mema_copy(g.mesh.vertices, sizeof(GeometryVertex) * g.mesh.vertices_num);
    g.mesh.indices = mema_copy(g.mesh.indices, sizeof(GeometryIndex) * g.mesh.indices_num);

    RendererResource geometry_res = {
        .type = RENDERER_RESOURCE_TYPE_GEOMETRY,
        .geometry = g
    };

    init_resource(rs, &geometry_res);
    return add_resource(rs->resource_handle_pool, &rs->da_resources, &geometry_res);
}

static void populate_constant_buffers(RendererState* rs, const RendererResourcePipeline* pl, const Mat4* model_matrix, const Mat4* mvp_matrix)
{
    const PipelineResource* pr = &resource_lookup(pl->res)->pipeline;

    for (u32 cb_idx = 0; cb_idx < pr->constant_buffers_num; ++cb_idx)
    {
        const ConstantBuffer* cb = pr->constant_buffers + cb_idx;

        u32 offset = 0;

        for (u32 cbf_idx = 0; cbf_idx < cb->fields_num; ++cbf_idx)
        {
            const ConstantBufferField* cbf = cb->fields + cbf_idx;

            switch(cbf->auto_value)
            {
                case CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL:
                    renderer_backend_update_constant_buffer(rs->rbs, pl->backend_state, cb->binding, model_matrix, sizeof(*model_matrix), offset);
                    break;

                case CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL_VIEW_PROJECTION:
                    renderer_backend_update_constant_buffer(rs->rbs, pl->backend_state, cb->binding, mvp_matrix, sizeof(*mvp_matrix), offset);
                    break;

                default: break;
            }

            offset += shader_data_type_size(cbf->type);
        }

    }
}

void renderer_draw(RendererState* rs, RendererResourceHandle pipeline_handle, RendererResourceHandle geometry_handle, const Vec3* cam_pos, const Quat* cam_rot)
{
    Mat4 camera_matrix = mat4_from_rotation_and_translation(cam_rot, cam_pos);
    Mat4 view_matrix = mat4_inverse(&camera_matrix);

    Mat4 model_matrix = mat4_identity();

    Vec2u size = renderer_backend_get_size(rs->rbs);
    Mat4 proj_matrix = mat4_create_projection_matrix(size.x, size.y);
    Mat4 proj_view_matrix = mat4_mul(&view_matrix, &proj_matrix);
    Mat4 mvp_matrix = mat4_mul(&model_matrix, &proj_view_matrix);

    const RendererResourcePipeline* pipeline = &get_resource(rs, pipeline_handle)->pipeline;
    populate_constant_buffers(rs, pipeline, &model_matrix, &mvp_matrix);
    renderer_backend_draw(rs->rbs, pipeline->backend_state, get_resource(rs, geometry_handle)->geometry.backend_state);
}

void renderer_present(RendererState* rs)
{
    renderer_backend_present(rs->rbs);
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

    for (size_t i = 0; i < array_num(rs->da_resources); ++i)
    {
        RendererResource* rr = rs->da_resources + i;

        if (rr->flag & RENDERER_RESOURCE_FLAG_SURFACE_SIZE_DEPENDENT)
            reinit_resource(rs, rr);
    }
    renderer_backend_wait_until_idle(rs->rbs);
}