// Phong vs Blinn-phong
#include "Common.hlsli"

#define NUM_DIR_LIGHTS 1
#define NUM_POINT_LIGHTS 1
#define NUM_SPOT_LIGHTS 1

Texture2D albedoTex : register(t0);

struct PixelShaderOutput
{
    float4 pixelColor : SV_Target0;
};

//cbuffer PixelConstantBuffer : register(b0)
//{
//    float3 eyeWorld;  -> Global
//    bool useTexture;  -> cbuffer
//    Material material; -> Material
//    Light light[MAX_LIGHTS]; -> Light
//    bool useBlinnPhong; -> cbuffer
//    float3 dummy;
//};

cbuffer PhongConstantBuffer : register(b3)
{
    int useBlinnPhong;
    bool useTexture;
    float2 dummy;
};

float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal,
                   float3 toEye)
{
    if (useBlinnPhong)
    {
        float3 halfway = normalize(toEye + lightVec);
        float hdotn = dot(halfway, normal);
        float3 specular = metallicFactor * pow(max(hdotn, 0.0f), shininess * 2.0);
        return albedoFactor + (roughnessFactor + specular) * lightStrength;
    }
    else
    {
        float3 r = -reflect(lightVec, normal);
        float3 specular = metallicFactor * pow(max(dot(toEye, r), 0.0f), shininess);
        return albedoFactor + (roughnessFactor + specular) * lightStrength;
    }
}

float3 ComputeDirectionalLight(Light L, float3 normal, float3 toEye)
{
    float3 lightVec = -L.direction;

    float ndotl = max(dot(lightVec, normal), 0.0f);
    float3 lightStrength = L.radiance * ndotl;

    // Luna DX12 책에서는 Specular 계산에도
    // Lambert's law가 적용된 lightStrength를 사용합니다.
    return BlinnPhong(lightStrength, lightVec, normal, toEye);
}

float CalcAttenuation(float d, float falloffStart, float falloffEnd)
{
    // Linear falloff
    return saturate((falloffEnd - d) / (falloffEnd - falloffStart));
}

float3 ComputePointLight(Light L, float3 pos, float3 normal,
                          float3 toEye)
{
    float3 lightVec = L.position - pos;

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);

    // 너무 멀면 조명이 적용되지 않음
    if (d > L.fallOffEnd)
    {
        return float3(0.0, 0.0, 0.0);
    }
    else
    {
        lightVec /= d;

        float ndotl = max(dot(lightVec, normal), 0.0f);
        float3 lightStrength = L.radiance * ndotl;

        float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        lightStrength *= att;

        return BlinnPhong(lightStrength, lightVec, normal, toEye);
    }
}

float3 ComputeSpotLight(Light L, float3 pos, float3 normal,
                         float3 toEye)
{
    float3 lightVec = L.position - pos;

    // 쉐이딩할 지점부터 조명까지의 거리 계산
    float d = length(lightVec);

    // 너무 멀면 조명이 적용되지 않음
    if (d > L.fallOffEnd)
    {
        return float3(0.0f, 0.0f, 0.0f);
    }
    else
    {
        lightVec /= d;

        float ndotl = max(dot(lightVec, normal), 0.0f);
        float3 lightStrength = L.radiance * ndotl;

        float att = CalcAttenuation(d, L.fallOffStart, L.fallOffEnd);
        lightStrength *= att;

        float spotFactor = pow(max(-dot(lightVec, L.direction), 0.0f), L.spotPower);
        lightStrength *= spotFactor;

        return BlinnPhong(lightStrength, lightVec, normal, toEye);
    }
    
    // if에 else가 없을 경우 경고 발생
    // warning X4000: use of potentially uninitialized variable
}

PixelShaderOutput main(PixelShaderInput input) : SV_TARGET
{
    PixelShaderOutput output;

    float3 toEye = normalize(eyeWorld - input.posWorld);

    float3 color = float3(0.0, 0.0, 0.0);
    
    int i = 0;
    
    // https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-for
    // https://forum.unity.com/threads/what-are-unroll-and-loop-when-to-use-them.1283096/
    
    [unroll] // warning X3557: loop only executes for 1 iteration(s), forcing loop to unroll
    for (i = 0; i < NUM_DIR_LIGHTS; ++i)
    {
        color += ComputeDirectionalLight(lights[i], input.normalWorld, toEye);
    }
    
    [unroll]
    for (i = NUM_DIR_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; ++i)
    {
        color += ComputePointLight(lights[i], input.posWorld, input.normalWorld, toEye);
    }
    
    [unroll]
    for (i = NUM_DIR_LIGHTS + NUM_POINT_LIGHTS; i < NUM_DIR_LIGHTS + NUM_POINT_LIGHTS + NUM_SPOT_LIGHTS; ++i)
    {
        color += ComputeSpotLight(lights[i], input.posWorld, input.normalWorld, toEye);
    }

    output.pixelColor = useTexture ? 
                float4(color, 1.0) * albedoTex.Sample(linearWrapSampler, input.texcoord) 
              : float4(color, 1.0);
    return output;
}
