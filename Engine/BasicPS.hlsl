#include "Common.hlsli"

Texture2D albedoTex : register(t0);

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    output.pixelColor = useAlbedoMap ? 
            3.0 * albedoTex.Sample(linearWrapSampler, input.texcoord)
            : float4(albedoFactor, 1.0);

    return output;
}