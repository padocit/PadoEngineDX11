#include "Common.hlsli"

Texture2D albedoTex : register(t0);

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    output.pixelColor = 2.0 * albedoTex.Sample(linearWrapSampler, input.texcoord);

    return output;
}