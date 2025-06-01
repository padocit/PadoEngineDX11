#include "Common.hlsli"

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    
    output.posProj = float4(input.position, 1.0);
    output.texcoord = input.texcoord;

    return output;
}