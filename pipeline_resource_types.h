#pragma once

#include "shader_resource_types.h"

fwd_handle(RendererResourceHandle);
fwd_enum(ShaderDataType);
fwd_handle(ResourceHandle);

typedef enum ConstantBufferAutoValue
{
    CONSTANT_BUFFER_AUTO_VALUE_NONE,
    CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL,
    CONSTANT_BUFFER_AUTO_VALUE_MAT_PROJECTION,
    CONSTANT_BUFFER_AUTO_VALUE_MAT_VIEW_PROJECTION,
    CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL_VIEW_PROJECTION
} ConstantBufferAutoValue;

typedef struct ConstantBufferField
{
    char* name;
    ShaderDataType type;
    ConstantBufferAutoValue auto_value;
} ConstantBufferField;

typedef enum VertexInputValue
{
    VERTEX_INPUT_VALUE_INVALID,
    VERTEX_INPUT_VALUE_POSITION,
    VERTEX_INPUT_VALUE_NORMAL,
    VERTEX_INPUT_VALUE_TEXCOORD,
    VERTEX_INPUT_VALUE_COLOR
} VertexInputValue;

typedef struct VertexInputField
{
    char* name;
    ShaderDataType type;
    VertexInputValue value;
} VertexInputField;

typedef struct ConstantBuffer
{
    ConstantBufferField* fields;
    u32 fields_num;
    u32 binding;
} ConstantBuffer;

typedef struct PipelineResource
{
    ResourceHandle* shader_stages;
    ConstantBuffer* constant_buffers;
    VertexInputField* vertex_input;
    u32 shader_stages_num;
    u32 vertex_input_num;
    u32 constant_buffers_num;
    RendererResourceHandle rrh;
} PipelineResource;
