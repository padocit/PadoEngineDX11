// Mipmap Ȯ�� ����
// ����: HLSL���� Mipmap_Level�� �������� �󼼵��� ���� Mipmap

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