cbuffer ConstantBuffer : register(b0)
{
    float4x4 model_view_projection;
    float4x4 model;
    float4x4 projection;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 vertex_world_pos : POSITION1;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

VOut VShader(float3 position : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD, float4 color : COLOR)
{
    VOut output;

    output.position = mul(model_view_projection, float4(position.xyz, 1));
    output.vertex_world_pos = float4(mul(position, (float3x3)model), 1);
    output.normal = normalize(mul((float3x3)model, normal));
    output.uv = uv;
    output.color = color;

    return output;
}

float4 PShader(float4 position : SV_POSITION, float4 vertex_world_pos : POSITION1, float3 normal : NORMAL, float2 uv : TEXCOORD, float4 color : COLOR) : SV_TARGET
{
    float3 sun = float3(29, -29, 35);
    float3 pos_to_sun = sun - vertex_world_pos.xyz;
    float l = max(dot(normalize(pos_to_sun), normal), 0.1f);
    return float4(color.rgb * l, 1);
}
