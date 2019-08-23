#include "renderer.h"
#include "window_types.h"
#include "renderer_backend.h"
#include "memory.h"
#include "handle_pool.h"
#include "array.h"
#include "debug.h"
#include "shader_resource.h"
#include "pipeline_resource.h"
#include "str.h"
#include "geometry_types.h"
#include "math.h"
#include <string.h>

typedef struct RendererResourceShader
{
    ShaderResource sr;
    RendererBackendShader* backend_state;
} RendererResourceShader;

typedef struct RendererResourcePipeline
{
    //RendererResourceHandle* shader_stages;
    //u32 shader_stages_num;
    PipelineResource pr;
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
    return renderer_backend_create_shader(rs->rbs, shader->sr.source, shader->sr.source_size);
}

static RendererBackendPipeline* pipeline_init(RendererState* rs, RendererResourcePipeline* pipeline)
{
    RendererBackendShader** backend_shader_stages = mema(sizeof(RendererBackendShader*) * pipeline->pr.shader_stages_num);
    ShaderType* backend_shader_types = mema(sizeof(ShaderType) * pipeline->pr.shader_stages_num);
    for (u32 shdr_idx = 0; shdr_idx < pipeline->pr.shader_stages_num; ++shdr_idx)
    {
        RendererResourceHandle shdr_handl = renderer_load_shader(rs, &pipeline->pr.shader_stages[shdr_idx]);
        RendererResourceShader* shader = &get_resource(rs, shdr_handl)->shader;
        backend_shader_stages[shdr_idx] = shader->backend_state;
        backend_shader_types[shdr_idx] = shader->sr.type;
    }

    ShaderDataType* vertex_input_types = mema(sizeof(ShaderDataType) * pipeline->pr.vertex_input_num);
    for (u32 vi_idx = 0; vi_idx < pipeline->pr.vertex_input_num; ++vi_idx)
    {
        vertex_input_types[vi_idx] = pipeline->pr.vertex_input[vi_idx].type;
    }

    u32* constant_buffer_sizes = mema(sizeof(u32) * pipeline->pr.constant_buffers_num);
    u32* constant_buffer_binding_indices = mema(sizeof(u32) * pipeline->pr.constant_buffers_num);

    for (u32 cb_idx = 0; cb_idx < pipeline->pr.constant_buffers_num; ++cb_idx)
    {
        u32 s = 0;

        for (u32 cb_field_idx = 0; cb_field_idx < pipeline->pr.constant_buffers[cb_idx].fields_num; ++cb_field_idx)
            s += shader_data_type_size(pipeline->pr.constant_buffers[cb_idx].fields[cb_field_idx].type);

        constant_buffer_sizes[cb_idx] = s;
        constant_buffer_binding_indices[cb_idx] = pipeline->pr.constant_buffers[cb_idx].binding;
    }

    RendererBackendPipeline* backend_state = renderer_backend_create_pipeline(
        rs->rbs, backend_shader_stages, backend_shader_types, pipeline->pr.shader_stages_num,
        vertex_input_types, pipeline->pr.vertex_input_num,
        constant_buffer_sizes, constant_buffer_binding_indices, pipeline->pr.constant_buffers_num);

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
            RendererResourceShader* s = &rr->shader;
            memf(s->sr.source);
        } break;
        
        case RENDERER_RESOURCE_TYPE_PIPELINE: {
            RendererResourcePipeline* p = &rr->pipeline;

            for (u32 i = 0; i < p->pr.constant_buffers_num; ++i)
            {
                for (u32 j = 0; j < p->pr.constant_buffers[i].fields_num; ++j)
                    memf(p->pr.constant_buffers[i].fields[j].name);

                memf(p->pr.constant_buffers[i].fields);
            }
            memf(p->pr.constant_buffers);

            for (u32 i = 0; i < p->pr.vertex_input_num; ++i)
                memf(p->pr.vertex_input[i].name);
            memf(p->pr.vertex_input);

            for (u32 i = 0; i < p->pr.shader_stages_num; ++i)
                memf(p->pr.shader_stages[i].source);
            memf(p->pr.shader_stages);

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

RendererResourceHandle renderer_load_shader(RendererState* rs, const ShaderResource* sr)
{
    RendererResource shader_res = {};
    shader_res.type = RENDERER_RESOURCE_TYPE_SHADER;
    RendererResourceShader* shader = &shader_res.shader;
    shader->sr = *sr;
    mema_repln(shader->sr.source, shader->sr.source_size);
    init_resource(rs, &shader_res);
    return add_resource(rs->resource_handle_pool, &rs->da_resources, &shader_res);
}

RendererResourceHandle renderer_load_pipeline(RendererState* rs, const PipelineResource* pr)
{
    RendererResource pipeline_res = {};
    pipeline_res.type = RENDERER_RESOURCE_TYPE_PIPELINE;
    RendererResourcePipeline* pipeline = &pipeline_res.pipeline;
    pipeline->pr = *pr;
    mema_repln(pipeline->pr.shader_stages, pipeline->pr.shader_stages_num);

    for (u32 i = 0; i < pipeline->pr.shader_stages_num; ++i)
        mema_repln(pipeline->pr.shader_stages[i].source, pipeline->pr.shader_stages[i].source_size);

    mema_repln(pipeline->pr.constant_buffers, pipeline->pr.constant_buffers_num);
    for (u32 i = 0; i < pipeline->pr.constant_buffers_num; ++i)
    {
        mema_repln(pipeline->pr.constant_buffers[i].fields, pipeline->pr.constant_buffers[i].fields_num);

        for (u32 j = 0; j <  pipeline->pr.constant_buffers[i].fields_num; ++j)
            mema_repls(pipeline->pr.constant_buffers[i].fields[j].name);
    }

    mema_repln(pipeline->pr.vertex_input, pipeline->pr.vertex_input_num);
    for (u32 i = 0; i < pipeline->pr.vertex_input_num; ++i)
        mema_repls(pipeline->pr.vertex_input[i].name);

    pipeline_res.flag = RENDERER_RESOURCE_FLAG_SURFACE_SIZE_DEPENDENT; // TODO, ask backend about this?
    init_resource(rs, &pipeline_res);
    return add_resource(rs->resource_handle_pool, &rs->da_resources, &pipeline_res);
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
    for (u32 cb_idx = 0; cb_idx < pl->pr.constant_buffers_num; ++cb_idx)
    {
        const ConstantBuffer* cb = pl->pr.constant_buffers + cb_idx;

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

    for (sizet i = 0; i < array_num(rs->da_resources); ++i)
    {
        RendererResource* rr = rs->da_resources + i;

        if (rr->flag & RENDERER_RESOURCE_FLAG_SURFACE_SIZE_DEPENDENT)
            reinit_resource(rs, rr);
    }
    renderer_backend_wait_until_idle(rs->rbs);
}