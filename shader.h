#pragma once

fwd_handle(RendererResourceHandle);
fwd_struct(RendererState);

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

typedef enum ShaderConstantBufferAutoValue
{
    SHADER_CONSTANT_BUFFER_AUTO_VALUE_NONE,
    SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL,
    SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_PROJECTION,
    SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_VIEW_PROJECTION,
    SHADER_CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL_VIEW_PROJECTION
} ShaderConstantBufferAutoValue;

typedef struct ShaderConstantBufferItem
{
    char* name;
    ShaderDataType type;
    ShaderConstantBufferAutoValue auto_value;
} ShaderConstantBufferItem;

typedef enum ShaderInputLayoutValue
{
    SHADER_INPUT_LAYOUT_VALUE_INVALID,
    SHADER_INPUT_LAYOUT_VALUE_VERTEX_POSITION,
    SHADER_INPUT_LAYOUT_VALUE_VERTEX_NORMAL,
    SHADER_INPUT_LAYOUT_VALUE_VERTEX_TEXCOORD,
    SHADER_INPUT_LAYOUT_VALUE_VERTEX_COLOR
} ShaderInputLayoutValue;

typedef struct ShaderInputLayoutItem
{
    char* name;
    ShaderDataType type;
    ShaderInputLayoutValue value;
} ShaderInputLayoutItem;

typedef struct ShaderConstantBuffer
{
    ShaderConstantBufferItem* items;
    u32 items_num;
    u32 binding;
} ShaderConstantBuffer;

typedef struct ShaderIntermediate
{
    char* source;
    u64 source_size;
    ShaderType type;
    ShaderConstantBuffer constant_buffer;
    ShaderInputLayoutItem* input_layout;
    u32 input_layout_num;
} ShaderIntermediate;

RendererResourceHandle shader_load(RendererState* rs, const char* filename);
u32 shader_data_type_size(ShaderDataType t);