// IBL + FresnelEffect
// Not HDRI !!

#include "Common.hlsli"

Texture2D albedoTex : register(t0);

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

cbuffer IblConstantBuffer : register(b3)
{
    float3 fresnelR0;
    float dummy;
};

// Schlick approximation: Eq. 9.17 in "Real-Time Rendering 4th Ed."
// fresnelR0�� ������ ���� ����
// Water : (0.02, 0.02, 0.02)
// Glass : (0.08, 0.08, 0.08)
// Plastic : (0.05, 0.05, 0.05)
// Gold: (1.0, 0.71, 0.29)
// Silver: (0.95, 0.93, 0.88)
// Copper: (0.95, 0.64, 0.54)
float3 SchlickFresnel(float3 fresnelR0, float3 normal, float3 toEye)
{
    // ���� �ڷ��
    // THE SCHLICK FRESNEL APPROXIMATION by Zander Majercik, NVIDIA
    // http://psgraphics.blogspot.com/2020/03/fresnel-equations-schlick-approximation.html
    
    float normalDotView = saturate(dot(normal, toEye));

    float f0 = 1.0f - normalDotView; // 90���̸� f0 = 1, 0���̸� f0 = 0

    // 1.0 ���� ���� ���� ���� �� ���ϸ� �� ���� ���� �˴ϴ�.
    // 0�� -> f0 = 0 -> fresnelR0 ��ȯ
    // 90�� -> f0 = 1.0 -> float3(1.0) ��ȯ
    // 0���� ����� �����ڸ��� Specular ����, 90���� ����� ������ ���� ����(fresnelR0)
    return fresnelR0 + (1.0f - fresnelR0) * pow(f0, 5.0);
}

PixelShaderOutput main(PixelShaderInput input)
{
    PixelShaderOutput output;
    
    float3 toEye = normalize(eyeWorld - input.posWorld);
    
    // ���� ������ �� �ִ� ������ �����Դϴ�.
    // IBL�� �ٸ� ���̵� ���(��: �� ���̵�)�� ���� ����� ���� �ֽ��ϴ�.
    // ����: https://www.shadertoy.com/view/lscBW4

    float4 diffuse = irradianceIBLTex.Sample(linearWrapSampler, input.normalWorld);
    float4 specular = specularIBLTex.Sample(linearWrapSampler, reflect(-toEye, input.normalWorld));

    diffuse.xyz *= roughnessFactor;
    specular.xyz *= pow((specular.x + specular.y + specular.z) / 3.0, shininess);
    specular.xyz *= metallicFactor;
       
    // ����: https://www.shadertoy.com/view/lscBW4
    float3 f = SchlickFresnel(fresnelR0, input.normalWorld, toEye);
    specular.xyz *= f;

    output.pixelColor = useAlbedoMap ?
            diffuse * albedoTex.Sample(linearWrapSampler, input.texcoord) + specular
            : diffuse + specular;

    return output;
}