struct VertexShaderInput
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangentModel : TANGENT;
};

struct VS_CONTROL_POINT_OUTPUT
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangentModel : TANGENT;

};

VS_CONTROL_POINT_OUTPUT main(VertexShaderInput input)
{
    VS_CONTROL_POINT_OUTPUT output;
    
    output.posModel = input.posModel;
    output.normalModel = input.normalModel;
    output.texcoord = input.texcoord;
    output.tangentModel = input.tangentModel;
    
    return output;
}