#include "Common.hlsli"

PixelShaderInput main(VertexShaderInput input)
{
    PixelShaderInput output;
    
    output.position = float4(input.position, 1.0);
    output.color = float4(input.color, 1.0);

    return output;
}