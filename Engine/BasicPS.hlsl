#include "Common.hlsli"

float4 main(PixelShaderInput input) : SV_Target
{
    return input.color;
}