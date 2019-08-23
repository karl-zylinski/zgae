#pragma once

fwd_handle(RendererResourceHandle);

typedef enum ShaderType
{
    SHADER_TYPE_INVALID,
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_FRAGMENT
} ShaderType;

typedef enum ShaderDataType
{
    SHADER_DATA_TYPE_INVALID,
    SHADER_DATA_TYPE_MAT4,
    SHADER_DATA_TYPE_VEC2,
    SHADER_DATA_TYPE_VEC3,
    SHADER_DATA_TYPE_VEC4,
} ShaderDataType;

typedef struct ShaderResource
{
    char* source;
    u64 source_size;
    ShaderType type;
} ShaderResource;