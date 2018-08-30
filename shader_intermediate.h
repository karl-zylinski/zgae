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
    const* char name;
    ShaderDataType type;
    ShaderAutoValue auto_value;
};

struct ShaderInputLayoutValue
{
    Invalid,
    VertexPosition,
    VertexNormal,
    VertexTexcoord,
    VertexColor
};

struct ShaderInputLayoutItem
{
    const* char name;
    ShaderDataType type;
    ShaderInputLayoutValue value;
};

struct ShaderIntermediate
{
    ShaderConstantBufferItem* D_constant_buffer;
    ShaderInputLayoutItem D_input_layout;
};