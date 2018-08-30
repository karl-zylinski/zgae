#pragma once

enum struct ShaderDataType
{
    Invalid,
    Mat4,
    Vec2,
    Vec3,
    Vec4
};

enum struct ShaderConstantBufferAutoValue
{
    None,
    MatMVP,
    MatModel,
    MatProjection
};

struct ShaderConstantBufferItem
{
    char* name;
    ShaderDataType type;
    ShaderConstantBufferAutoValue auto_value;
};

enum struct ShaderInputLayoutValue
{
    Invalid,
    VertexPosition,
    VertexNormal,
    VertexTexCoord,
    VertexColor
};

struct ShaderInputLayoutItem
{
    char* name;
    ShaderDataType type;
    ShaderInputLayoutValue value;
};

struct ShaderIntermediate
{
    char* source;
    size_t source_size;
    ShaderConstantBufferItem* constant_buffer;
    ShaderInputLayoutItem* input_layout;
    unsigned constant_buffer_size;
    unsigned input_layout_size;
};

static unsigned shader_data_type_size(ShaderDataType type)
{
    switch (type)
    {
        case ShaderDataType::Mat4: return 64;
        case ShaderDataType::Vec2: return 8;
        case ShaderDataType::Vec3: return 12;
        case ShaderDataType::Vec4: return 16;
    }

    Error("Trying to get size of invalid ShaderDataType type");
    return 0;
}