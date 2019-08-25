#pragma once


typedef enum RenderResourceType
{
    RENDER_RESOURCE_TYPE_INVALID,
    RENDER_RESOURCE_TYPE_SHADER,
    RENDER_RESOURCE_TYPE_PIPELINE,
    RENDER_RESOURCE_TYPE_MESH,
    RENDER_RESOURCE_TYPE_NUM
} RenderResourceType;

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
