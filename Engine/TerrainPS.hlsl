#include "Common.hlsli"

// �����ڷ�
// https://github.com/Nadrin/PBR/blob/master/data/shaders/hlsl/pbr.hlsl

// �޽� ���� �ؽ���� t0 ���� ����
Texture2D albedoTex : register(t0);
Texture2D normalTex : register(t1);
Texture2D aoTex : register(t2);
Texture2D metallicRoughnessTex : register(t3);
Texture2D emissiveTex : register(t4);

static const float3 Fdielectric = 0.04; // ��ݼ�(Dielectric) ������ F0

float3 SchlickFresnel(float3 F0, float NdotH)
{
    return F0 + (1.0 - F0) * pow(2.0, (-5.55473 * NdotH - 6.98316) * NdotH);
    //return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 GetNormal(PixelShaderInput input)
{
    float3 normalWorld = normalize(input.normalWorld);
    
    if (useNormalMap) // NormalWorld�� ��ü
    {
        float3 normal = normalTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias).rgb;
        normal = 2.0 * normal - 1.0; // ���� ���� [-1.0, 1.0]

        // OpenGL �� ��ָ��� ��쿡�� y ������ �������ݴϴ�.
        normal.y = invertNormalMapY ? -normal.y : normal.y;
        
        float3 N = normalWorld;
        float3 T = normalize(input.tangentWorld - dot(input.tangentWorld, N) * N);
        float3 B = cross(N, T);
        
        // matrix�� float4x4, ���⼭�� ���� ��ȯ���̶� 3x3 ���
        float3x3 TBN = float3x3(T, B, N);
        normalWorld = normalize(mul(normal, TBN));
    }
    
    return normalWorld;
}

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

PixelShaderOutput main(PixelShaderInput input)
{
    float3 pixelToEye = normalize(eyeWorld - input.posWorld);
    float3 normalWorld = GetNormal(input);
    
    float4 albedo = useAlbedoMap ? albedoTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias) * float4(albedoFactor, 1)
                                 : float4(albedoFactor, 1);
    float ao = useAOMap ? aoTex.SampleLevel(linearWrapSampler, input.texcoord, lodBias).r : 1.0;
    
    float4 diffuse = irradianceIBLTex.Sample(linearWrapSampler, normalWorld);
    float4 specular = specularIBLTex.Sample(linearWrapSampler, reflect(-pixelToEye, normalWorld));

    diffuse.xyz *= roughnessFactor;
    specular.xyz *= pow((specular.x + specular.y + specular.z) / 3.0, shininess);
    specular.xyz *= metallicFactor;

    float3 F0 = lerp(Fdielectric, albedo.xyz, metallicFactor);
    float3 F = SchlickFresnel(F0, max(0.0, dot(normalWorld, pixelToEye)));
    specular.xyz *= F;

    PixelShaderOutput output;
    output.pixelColor = albedo * diffuse * ao + specular;
    
    return output;
}