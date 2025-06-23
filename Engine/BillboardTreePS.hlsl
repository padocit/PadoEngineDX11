#include "Common.hlsli"

Texture2D g_texture : register(t0);

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

struct BillboardPixelShaderInput
{
    float4 pos : SV_POSITION; // Screen position
    float4 posWorld : POSITION0;
    float4 center : POSITION1;
    float2 texCoord : TEXCOORD;
    uint primID : SV_PrimitiveID;
};

PixelShaderOutput main(BillboardPixelShaderInput input)
{
    float4 pixelColor = g_texture.Sample(linearWrapSampler, input.texCoord);
    clip((pixelColor.a < 0.9f) || (pixelColor.r + pixelColor.g + pixelColor.b) > 1.5 ?
        -1 : 1);
    
    PixelShaderOutput output;
    output.pixelColor = float4(pixelColor.xyz * 0.6, 1.0);

    return output;
}
