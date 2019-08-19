#include "renderer.h"
#include "window_types.h"
#include "renderer_backend.h"
#include "memory.h"

fwd_struct(renderer_backend_state_t);

typedef struct renderer_state_t
{
    renderer_backend_state_t* rbs;
} renderer_state_t;

renderer_state_t* renderer_create(window_type_t window_type, void* window_data)
{
    renderer_backend_state_t* rbs = renderer_backend_create(window_type, window_data);
    renderer_state_t* rs = mema_zero(sizeof(renderer_state_t));
    rs->rbs = rbs;
    return rs;
}

void renderer_destroy(renderer_state_t* rs)
{
    renderer_backend_destroy(rs->rbs);
    memf(rs);
}

renderer_resource_handle_t renderer_load_shader(renderer_state_t* rs, const shader_intermediate_t* si)
{
    return renderer_backend_load_shader(rs->rbs, si);
}

renderer_resource_handle_t renderer_load_pipeline(renderer_state_t* rs, const pipeline_intermediate_t* pi)
{
    return renderer_backend_load_pipeline(rs->rbs, pi);
}

renderer_resource_handle_t renderer_load_geometry(renderer_state_t* rs, const geometry_vertex_t* vertices, uint32_t vertices_num)
{
    return renderer_backend_load_geometry(rs->rbs, vertices, vertices_num);
}

void renderer_draw(renderer_state_t* rs, renderer_resource_handle_t pipeline_handle, renderer_resource_handle_t geometry_handle)
{
    renderer_backend_draw(rs->rbs, pipeline_handle, geometry_handle);
}

void renderer_present(renderer_state_t* rs)
{
    renderer_backend_present(rs->rbs);
}

void renderer_update_constant_buffer(renderer_state_t* rs, renderer_resource_handle_t pipeline_handle, uint32_t binding, void* data, uint32_t data_size)
{
    renderer_backend_update_constant_buffer(rs->rbs, pipeline_handle, binding, data, data_size);
}

void renderer_wait_for_new_frame(renderer_state_t* rs)
{
    renderer_backend_wait_for_new_frame(rs->rbs);
}