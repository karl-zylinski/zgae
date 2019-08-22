#pragma once

fwd_handle(renderer_resource_handle_t);

typedef struct pipeline_intermediate_t
{
    renderer_resource_handle_t* shader_stages;
    uint32_t shader_stages_num;
} pipeline_intermediate_t;
