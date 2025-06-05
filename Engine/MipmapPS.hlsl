// Mipmap 확인 예제
// 주의: HLSL에서 Mipmap_Level이 높을수록 상세도가 낮은 Mipmap

#include "Common.hlsli"

Texture2D albedoTex : register(t0);

// cbuffer (b3)
cbuffer MipmapConstantBuffer : register(b3)
{
    float lod;
    float3 dummy;
}

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    output.pixelColor = useAlbedoMap ?
            albedoTex.SampleLevel(linearWrapSampler, input.texcoord, lod) 
            : float4(albedoFactor, 1.0);

    return output;
}