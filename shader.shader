cbuffer ConstantBuffer : register(b0)
{
    float4x4 model_view_projection;
    float4x4 model;
    float4x4 projection;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 vertex_world_pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

VOut VShader(float4 position : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD, float4 color : COLOR)
{
    VOut output;

    output.position = mul(model_view_projection, position);
    output.vertex_world_pos = mul(model, position);
    output.normal = normal;
    output.uv = uv;
    output.color = color;

    return output;
}


float4 PShader(float4 position : SV_POSITION, float4 vertex_world_pos : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD, float4 color : COLOR) : SV_TARGET
{
    return float4(color.rgb, 1);
}
