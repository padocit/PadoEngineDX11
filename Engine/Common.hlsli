#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

// 샘플러들을 모든 쉐이더에서 공통으로 사용
SamplerState linearWrapSampler : register(s0);
SamplerState linearClampSampler : register(s1);
SamplerState shadowPointSampler : register(s2);
SamplerComparisonState shadowCompareSampler : register(s3);
SamplerState pointWrapSampler : register(s4);
SamplerState linearMirrorSampler : register(s5);
SamplerState pointClampSampler : register(s6);

// 공용 Constants
cbuffer GlobalConstants : register(b0)
{
    matrix view;
    matrix proj;
    matrix invProj; // 역프로젝션행렬
    matrix viewProj;
    matrix invViewProj; // Proj -> World
    matrix invView;

    float3 eyeWorld;
    float strengthIBL;

    int textureToDraw = 0; // 0: Env, 1: Specular, 2: Irradiance, 그외: 검은색
    float envLodBias = 0.0f; // 환경맵 LodBias
    float lodBias = 2.0f; // 다른 물체들 LodBias
    float globalTime;
    
    //Light lights[MAX_LIGHTS];
};

cbuffer MeshConstants : register(b1)
{
    matrix world; // Model(또는 Object) 좌표계 -> World로 변환
    matrix worldIT; // World의 InverseTranspose
    matrix worldInv;
    int useHeightMap;
    float heightScale;
    float windTrunk;
    float windLeaves;
};

cbuffer MaterialConstants : register(b2)
{
    float3 albedoFactor; // baseColor
    float roughnessFactor;
    float metallicFactor;
    float3 emissionFactor;

    int useAlbedoMap;
    int useNormalMap;
    int useAOMap; // Ambient Occlusion
    int invertNormalMapY;
    int useMetallicMap;
    int useRoughnessMap;
    int useEmissiveMap;
    float dummy2;
};

struct VertexShaderInput
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangentModel : TANGENT;
};

struct PixelShaderInput
{
    float4 posProj : SV_POSITION; // Screen position
    float3 posWorld : POSITION0;  // World position (Lighting)
    float3 posModel : POSITION1;  // Volume casting
    float3 normalWorld : NORMAL0;
    float2 texcoord : TEXCOORD0;
    float3 tangentWorld : TANGENT0;
};

#endif