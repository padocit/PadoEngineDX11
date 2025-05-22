#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

struct VertexShaderInput
{
    float3 position : POSITION;
    float3 color : COLOR;
};

struct PixelShaderInput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

#endif