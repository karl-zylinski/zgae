cbuffer ConstantBuffer : register(b0)
{
    float4x4 model_view_projection;
    float4x4 model;
    float4x4 projection;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 vertex_pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

VOut VShader(float4 position : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD, float4 color : COLOR)
{
    VOut output;

    output.position = mul(model_view_projection, position);
    output.vertex_pos = position;
    output.normal = normal;
    output.uv = uv;
    output.color = color;

    return output;
}

Texture2D lightmap;
SamplerState lightmap_ss;

float4 PShader(float4 position : SV_POSITION, float4 vertex_pos : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD, float4 color : COLOR) : SV_TARGET
{
    return lightmap.Sample(lightmap_ss, uv);
}
