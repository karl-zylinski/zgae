#pragma once


enum struct RenderResourceType
{
    Invalid, Shader, Pipeline, Mesh, World, Num
};

enum struct ShaderType
{
    Invalid, Vertex, Fragment
};

enum struct ShaderDataType
{
    Invalid, Mat4, Vec2, Vec3, Vec4
};

enum struct ConstantBufferAutoValue
{
    None, MatModel, MatProjection, MatViewProjection,MatModelViewProjection
};

struct ConstantBufferField
{
    char* name;
    ShaderDataType type;
    ConstantBufferAutoValue auto_value;
};

enum struct VertexInputValue
{
    Invalid, Position, Normal, Texcoord, Color
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
