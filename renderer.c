#include "renderer.h"
#include "window_types.h"
#include "renderer_backend.h"
#include "memory.h"
#include "handle_pool.h"
#include "array.h"
#include "debug.h"
#include "pipeline.h"
#include "shader.h"

fwd_struct(renderer_backend_state_t);
fwd_struct(renderer_backend_shader_t);
fwd_struct(renderer_backend_pipeline_t);
fwd_struct(renderer_backend_geometry_t);

typedef struct renderer_resource_shader_t
{
    char* source;
    uint64_t source_size;
    shader_type_t type;
    shader_constant_buffer_t constant_buffer;
    shader_input_layout_item_t* input_layout;
    uint32_t input_layout_num;
    renderer_backend_shader_t* backend_state;
} renderer_resource_shader_t;

typedef struct renderer_resource_pipeline_t
{
    renderer_resource_handle_t* shader_stages;
    uint32_t shader_stages_num;
    renderer_backend_pipeline_t* backend_state;
} renderer_resource_pipeline_t;

typedef struct renderer_resource_geometry_t
{
    // TODO ADD VERTICEs ETC HERE
    renderer_backend_geometry_t* backend_state;
} renderer_resource_geometry_t;

typedef enum renderer_resource_type_t
{
    RENDERER_RESOURCE_TYPE_INVALID,
    RENDERER_RESOURCE_TYPE_SHADER,
    RENDERER_RESOURCE_TYPE_PIPELINE,
    RENDERER_RESOURCE_TYPE_GEOMETRY,
    RENDERER_RESOURCE_TYPE_NUM
} renderer_resource_type_t;

typedef struct renderer_resource_t
{
    renderer_resource_type_t type;
    renderer_resource_handle_t handle;

    union
    {
        renderer_resource_shader_t shader;
        renderer_resource_pipeline_t pipeline;
        renderer_resource_geometry_t geometry;
    };
} renderer_resource_t;

typedef struct renderer_state_t
{
    renderer_backend_state_t* rbs;
    renderer_resource_t* da_resources;
    handle_pool_t* resource_handle_pool;
} renderer_state_t;

renderer_state_t* renderer_create(window_type_t window_type, void* window_data)
{
    renderer_state_t* rs = mema_zero(sizeof(renderer_state_t));
    rs->resource_handle_pool = handle_pool_create();

    for (renderer_resource_type_t t = 1; t < RENDERER_RESOURCE_TYPE_NUM; ++t)
        handle_pool_set_type(rs->resource_handle_pool, t, stringify(t));
    
    renderer_backend_state_t* rbs = renderer_backend_create(window_type, window_data);
    rs->rbs = rbs;
    return rs;
}

static void destroy_resource(renderer_state_t* rs, renderer_resource_t* rr)
{
    switch(rr->type)
    {
        case RENDERER_RESOURCE_TYPE_SHADER: {
            renderer_resource_shader_t* s = &rr->shader;
            renderer_backend_destroy_shader(rs->rbs, s->backend_state);
            memf(s->source);
            memf(s->constant_buffer.items);
            memf(s->input_layout);
        } break;
        
        case RENDERER_RESOURCE_TYPE_PIPELINE: {
            renderer_resource_pipeline_t* p = &rr->pipeline;
            renderer_backend_destroy_pipeline(rs->rbs, p->backend_state);
            memf(p->shader_stages);
        } break;

        case RENDERER_RESOURCE_TYPE_GEOMETRY: {
            renderer_backend_destroy_geometry(rs->rbs, rr->geometry.backend_state);
        } break;

        case RENDERER_RESOURCE_TYPE_NUM:
        case RENDERER_RESOURCE_TYPE_INVALID:
            error("Invalid resource in renderer resource list"); break;
    }

    handle_pool_return(rs->resource_handle_pool, rr->handle);
}

static void destroy_all_resources(renderer_state_t* rs)
{
    info("Destroying all renderer resources");

    for (size_t i = 0; i < array_num(rs->da_resources); ++i)
    {
        destroy_resource(rs, rs->da_resources + i);
    }
}

void renderer_destroy(renderer_state_t* rs)
{
    info("Destroying renderer");
    renderer_backend_wait_until_idle(rs->rbs);
    destroy_all_resources(rs);
    array_destroy(rs->da_resources);
    renderer_backend_destroy(rs->rbs);
    handle_pool_destroy(rs->resource_handle_pool);
    memf(rs);
}

static renderer_resource_handle_t add_resource(handle_pool_t* hp, renderer_resource_t** da_resources, renderer_resource_t* res)
{
    renderer_resource_handle_t h = handle_pool_reserve(hp, res->type);
    res->handle = h;
    array_fill_and_set(*da_resources, handle_index(h), *res);
    return h;
}

static renderer_resource_t* get_resource(renderer_state_t* rs, renderer_resource_handle_t h)
{
    renderer_resource_t* rr = rs->da_resources + handle_index(h);
    check_slow(handle_type(h) == rr->type, "Handle points to resource of wrong type");
    return rr;
}

static renderer_backend_shader_t* shader_init(renderer_backend_state_t* rbs, const renderer_resource_shader_t* shader)
{
    return renderer_backend_load_shader(rbs, shader->source, shader->source_size);
}

renderer_resource_handle_t renderer_load_shader(renderer_state_t* rs, const shader_intermediate_t* si)
{
    renderer_resource_t shader_res = {};
    shader_res.type = RENDERER_RESOURCE_TYPE_SHADER;
    renderer_resource_shader_t* shader = &shader_res.shader;
    shader->source = mema_copy(si->source, si->source_size);
    shader->source_size = si->source_size;
    shader->type = si->type;
    shader->constant_buffer.items = mema_copy(si->constant_buffer.items, si->constant_buffer.items_num * sizeof(shader_constant_buffer_item_t));
    shader->constant_buffer.items_num = si->constant_buffer.items_num;
    shader->constant_buffer.binding = si->constant_buffer.binding;
    shader->input_layout = mema_copy(si->input_layout, si->input_layout_num * sizeof(shader_input_layout_item_t));
    shader->input_layout_num = si->input_layout_num;
    shader->backend_state = shader_init(rs->rbs, shader);

    return add_resource(rs->resource_handle_pool, &rs->da_resources, &shader_res);
}

static renderer_backend_pipeline_t* pipeline_init(renderer_state_t* rs, const renderer_resource_pipeline_t* pipeline)
{
    renderer_backend_shader_t** backend_shader_stages = mema(sizeof(renderer_backend_shader_t*) * pipeline->shader_stages_num);
    shader_type_t* backend_shader_types = mema(sizeof(shader_type_t) * pipeline->shader_stages_num);
    shader_data_type_t* vertex_input_types = NULL;
    uint32_t vertex_input_types_num = 0;
    uint32_t* da_constant_buffer_sizes = NULL;
    uint32_t* da_constant_buffer_binding_indices = NULL;

    for (uint32_t shdr_idx = 0; shdr_idx < pipeline->shader_stages_num; ++shdr_idx)
    {
        renderer_resource_shader_t* shader = &get_resource(rs, pipeline->shader_stages[shdr_idx])->shader;
        backend_shader_stages[shdr_idx] = shader->backend_state;
        backend_shader_types[shdr_idx] = shader->type;

        if (shader->input_layout_num > 0)
        {
            check(!vertex_input_types, "Pipeline contains multiple shader that specify vertex shader inputs");
            vertex_input_types_num = shader->input_layout_num;
            vertex_input_types = mema(sizeof(shader_data_type_t) * vertex_input_types_num);

            for (uint32_t ipt_idx = 0; ipt_idx < vertex_input_types_num; ++ipt_idx)
                vertex_input_types[ipt_idx] = shader->input_layout[ipt_idx].type;
        }

        if (shader->constant_buffer.items_num > 0)
        {
            uint32_t s = 0;

            for (uint32_t cb_item_idx = 0; cb_item_idx < shader->constant_buffer.items_num; ++cb_item_idx)
                s += shader_data_type_size(shader->constant_buffer.items[cb_item_idx].type);

            array_add(da_constant_buffer_sizes, s);
            array_add(da_constant_buffer_binding_indices, shader->constant_buffer.binding);
        }
    }

    uint32_t constant_buffers_num = array_num(da_constant_buffer_sizes);

    check(vertex_input_types, "Pipeline has no shader that specifies vertex inputs");

    renderer_backend_pipeline_t* backend_state = renderer_backend_load_pipeline(
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

renderer_resource_handle_t renderer_load_pipeline(renderer_state_t* rs, const pipeline_intermediate_t* pi)
{
    renderer_resource_t pipeline_res = {};
    pipeline_res.type = RENDERER_RESOURCE_TYPE_PIPELINE;
    renderer_resource_pipeline_t* pipeline = &pipeline_res.pipeline;
    pipeline->shader_stages = mema_copy(pi->shader_stages, sizeof(renderer_resource_handle_t) * pi->shader_stages_num);
    pipeline->shader_stages_num = pi->shader_stages_num;
    pipeline->backend_state = pipeline_init(rs, pipeline);
    return add_resource(rs->resource_handle_pool, &rs->da_resources, &pipeline_res);
}

renderer_resource_handle_t renderer_load_geometry(renderer_state_t* rs, const geometry_vertex_t* vertices, uint32_t vertices_num)
{
    renderer_resource_t geometry_res = {};
    geometry_res.type = RENDERER_RESOURCE_TYPE_GEOMETRY;
    renderer_resource_geometry_t* geometry = &geometry_res.geometry;
    // TODO: Do init thing like above here, and put vertices and stuff into renderer_resource so we can re-init later.

    geometry->backend_state = renderer_backend_load_geometry(rs->rbs, vertices, vertices_num);
    return add_resource(rs->resource_handle_pool, &rs->da_resources, &geometry_res);
}

void renderer_draw(renderer_state_t* rs, renderer_resource_handle_t pipeline_handle, renderer_resource_handle_t geometry_handle)
{
    renderer_backend_draw(rs->rbs, get_resource(rs, pipeline_handle)->pipeline.backend_state, get_resource(rs, geometry_handle)->geometry.backend_state);
}

void renderer_present(renderer_state_t* rs)
{
    renderer_backend_present(rs->rbs);
}

void renderer_update_constant_buffer(renderer_state_t* rs, renderer_resource_handle_t pipeline_handle, uint32_t binding, void* data, uint32_t data_size)
{
    renderer_backend_update_constant_buffer(rs->rbs, get_resource(rs, pipeline_handle)->pipeline.backend_state, binding, data, data_size);
}

void renderer_wait_for_new_frame(renderer_state_t* rs)
{
    renderer_backend_wait_for_new_frame(rs->rbs);
}