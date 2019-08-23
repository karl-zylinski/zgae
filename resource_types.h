#pragma once

#include "shader_resource_types.h"
#include "pipeline_resource_types.h"

fwd_handle(ResourceHandle);

typedef enum ResourceType
{
    RESOURCE_TYPE_INVALID,
    RESOURCE_TYPE_SHADER,
    RESOURCE_TYPE_PIPELINE,
    RESOURCE_TYPE_NUM
} ResourceType;

typedef struct Resource
{
    ResourceType type;
    ResourceHandle handle;

    union
    {
        ShaderResource shader;
        PipelineResource pipeline;
    };
} Resource;