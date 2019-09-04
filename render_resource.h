#pragma once

enum RenderResourceType
{
    RENDER_RESOURCE_TYPE_INVALID,
    RENDER_RESOURCE_TYPE_SHADER,
    RENDER_RESOURCE_TYPE_PIPELINE,
    RENDER_RESOURCE_TYPE_MESH,
    RENDER_RESOURCE_TYPE_WORLD,
    RENDER_RESOURCE_TYPE_NUM
};

enum ShaderType : u32
{
    SHADER_TYPE_INVALID,
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_FRAGMENT
};

enum ShaderDataType : u32
{
    SHADER_DATA_TYPE_INVALID,
    SHADER_DATA_TYPE_MAT4,
    SHADER_DATA_TYPE_VEC2,
    SHADER_DATA_TYPE_VEC3,
    SHADER_DATA_TYPE_VEC4,
};

enum ConstantBufferAutoValue
{
    CONSTANT_BUFFER_AUTO_VALUE_NONE,
    CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL,
    CONSTANT_BUFFER_AUTO_VALUE_MAT_PROJECTION,
    CONSTANT_BUFFER_AUTO_VALUE_MAT_VIEW_PROJECTION,
    CONSTANT_BUFFER_AUTO_VALUE_MAT_MODEL_VIEW_PROJECTION
};

enum VertexInputValue
{
    VERTEX_INPUT_VALUE_INVALID,
    VERTEX_INPUT_VALUE_POSITION,
    VERTEX_INPUT_VALUE_NORMAL,
    VERTEX_INPUT_VALUE_TEXCOORD,
    VERTEX_INPUT_VALUE_COLOR
};

struct ConstantBufferField
{
    char* name;
    ShaderDataType type;
    ConstantBufferAutoValue auto_value;
};

struct VertexInputField
{
    char* name;
    ShaderDataType type;
    VertexInputValue value;
};

struct ConstantBuffer
{
    ConstantBufferField* fields;
    u32 fields_num;
    u32 binding;
};

u32 shader_data_type_size(ShaderDataType t);