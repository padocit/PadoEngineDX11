#include "Common.hlsli"

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    
    output.posModel = input.posModel;
    output.posWorld = mul(float4(output.posModel, 1.0), world).xyz;
    output.posProj = mul(float4(output.posWorld, 1.0), viewProj); 
    
    output.texcoord = input.texcoord;
    
    output.normalWorld = mul(float4(input.normalModel, 0.0), worldIT).xyz;
    output.normalWorld = normalize(output.normalWorld);
    output.tangentWorld = mul(float4(input.tangentModel, 0.0f), world).xyz;

    return output;
}