#include "Common.hlsli"

cbuffer PixelConstants : register(b0)
{
    float alpha;
    float3 dummy;
}

float4 main(PixelShaderInput input) : SV_Target
{
    return float4(input.color.xyz * alpha, 1.0);
}