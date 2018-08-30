cbuffer ConstantBuffer : register(b0)
{
    float4x4 view_projection;
};

struct VOut
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VOut VShader(float3 position : POSITION, float4 color : COLOR)
{
    VOut output;
    output.position = mul(view_projection, float4(position.xyz, 1));
    output.color = color;
    return output;
}

float4 PShader(float4 position : SV_POSITION, float4 color : COLOR) : SV_TARGET
{
    return float4(color.rgb, 1);
}
