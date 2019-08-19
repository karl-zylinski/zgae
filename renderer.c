#include "renderer.h"
#include "window_types.h"
#include "renderer_backend.h"
#include "memory.h"
#include "renderer_resource_types.h"
#include "handle_pool.h"
#include "array.h"
#include "debug.h"
#include "pipeline.h"

fwd_struct(renderer_backend_state_t);

typedef struct renderer_resource_t
{
    renderer_resource_type_t type;
    renderer_resource_handle_t handle;
    void* backend_state;
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

static void destroy_resource(renderer_backend_state_t* rbs, renderer_resource_t* rr)
{
    renderer_backend_destroy_resource(rbs, rr->type, rr->backend_state);
}

static void destroy_all_resources(renderer_backend_state_t* rbs, handle_pool_t* hp, renderer_resource_t* rrs, size_t rrs_n)
{
    for (size_t i = 0; i < rrs_n; ++i)
    {
        renderer_resource_t* rr = rrs + i;
        destroy_resource(rbs, rr);
        handle_pool_return(hp, rr->handle);
    }
}

void renderer_destroy(renderer_state_t* rs)
{
    destroy_all_resources(rs->rbs, rs->resource_handle_pool, rs->da_resources, array_num(rs->da_resources));
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

renderer_resource_handle_t renderer_load_shader(renderer_state_t* rs, const shader_intermediate_t* si)
{
    renderer_resource_t shader_res = {};
    shader_res.type = RENDERER_RESOURCE_TYPE_SHADER;
    shader_res.backend_state = renderer_backend_load_shader(rs->rbs, si);
    return add_resource(rs->resource_handle_pool, &rs->da_resources, &shader_res);
}

renderer_resource_handle_t renderer_load_pipeline(renderer_state_t* rs, const pipeline_intermediate_t* pi)
{
    renderer_resource_t pipeline_res = {};
    pipeline_res.type = RENDERER_RESOURCE_TYPE_PIPELINE;

    void** shader_stage_backend_states = mema(sizeof(void*) * pi->shader_stages_num);
    for (uint32_t i = 0; i < pi->shader_stages_num; ++i)
    {
        renderer_resource_t* sr = get_resource(rs, pi->shader_stages[i]);
        shader_stage_backend_states[i] = sr->backend_state;
    }

    pipeline_res.backend_state = renderer_backend_load_pipeline(rs->rbs, shader_stage_backend_states, pi->shader_stages_num);
    memf(shader_stage_backend_states);
    return add_resource(rs->resource_handle_pool, &rs->da_resources, &pipeline_res);
}

renderer_resource_handle_t renderer_load_geometry(renderer_state_t* rs, const geometry_vertex_t* vertices, uint32_t vertices_num)
{
    renderer_resource_t geometry_res = {};
    geometry_res.type = RENDERER_RESOURCE_TYPE_GEOMETRY;
    geometry_res.backend_state = renderer_backend_load_geometry(rs->rbs, vertices, vertices_num);
    return add_resource(rs->resource_handle_pool, &rs->da_resources, &geometry_res);
}

void renderer_draw(renderer_state_t* rs, renderer_resource_handle_t pipeline_handle, renderer_resource_handle_t geometry_handle)
{
    renderer_backend_draw(rs->rbs, get_resource(rs, pipeline_handle)->backend_state, get_resource(rs, geometry_handle)->backend_state);
}

void renderer_present(renderer_state_t* rs)
{
    renderer_backend_present(rs->rbs);
}

void renderer_update_constant_buffer(renderer_state_t* rs, renderer_resource_handle_t pipeline_handle, uint32_t binding, void* data, uint32_t data_size)
{
    renderer_backend_update_constant_buffer(rs->rbs, get_resource(rs, pipeline_handle)->backend_state, binding, data, data_size);
}

void renderer_wait_for_new_frame(renderer_state_t* rs)
{
    renderer_backend_wait_for_new_frame(rs->rbs);
}