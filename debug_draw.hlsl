cbuffer ConstantBuffer : register(b0)
{
    float4x4 view_projection;
    float4 color;
};

struct VOut
{
    float4 position : SV_POSITION;
};

VOut VShader(float3 position : POSITION)
{
    VOut output;
    output.position = mul(view_projection, float4(position.xyz, 1));
    return output;
}

float4 PShader(float4 position : SV_POSITION) : SV_TARGET
{
    return float4(color.rgb, 1);
}
