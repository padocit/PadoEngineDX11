#include "Common.hlsli"

cbuffer PixelConstants : register(b0)
{
    float alpha;
    float3 dummy;
}

float4 main(PixelShaderInput input) : SV_Target
{
    return float4(0.0, 1.0, 0.0, 1.0);
}