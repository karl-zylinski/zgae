#pragma once

fwd_handle(RendererResourceHandle);

typedef struct PipelineIntermediate
{
    RendererResourceHandle* shader_stages;
    u32 shader_stages_num;
} PipelineIntermediate;
