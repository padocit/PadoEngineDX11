#ifndef __COMMON_HLSLI__
#define __COMMON_HLSLI__

struct VertexShaderInput
{
    float3 position : POSITION;
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