#include "Common.hlsli"

struct VS_CONTROL_POINT_OUTPUT
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangentModel : TANGENT;

};

struct HS_CONTROL_POINT_OUTPUT
{
    float3 posModel : POSITION;
    float3 normalModel : NORMAL;
    float2 texcoord : TEXCOORD;
    float3 tangentModel : TANGENT;
};

struct HS_CONSTANT_DATA_OUTPUT
{
	float edgeTessFactor[4] : SV_TessFactor;
	float insideTessFactor[2] : SV_InsideTessFactor;
};

HS_CONSTANT_DATA_OUTPUT CalcHSPatchConstants(
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip,
	uint patchID : SV_PrimitiveID)
{
    float3 center = 
        (ip[0].posModel + ip[1].posModel + ip[2].posModel + ip[3].posModel).xyz * 0.25;
    center = mul(float4(center, 1.0), world).xyz;
    float dist = length(center - eyeWorld);
    float minDist = 1.0;
    float maxDist = 10.0;
    float tess = 63.0 * saturate((maxDist - dist) / (maxDist - minDist)) + 1.0;
    
    HS_CONSTANT_DATA_OUTPUT output;
    
    output.edgeTessFactor[0] = tess;
    output.edgeTessFactor[1] = tess;
    output.edgeTessFactor[2] = tess;
    output.edgeTessFactor[3] = tess;
    output.insideTessFactor[0] = tess;
    output.insideTessFactor[1] = tess;
	
    return output;
}

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CalcHSPatchConstants")]
[maxtessfactor(64.0f)]
HS_CONTROL_POINT_OUTPUT main( 
	InputPatch<VS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> ip, 
	uint i : SV_OutputControlPointID,
	uint patchID : SV_PrimitiveID )
{
	HS_CONTROL_POINT_OUTPUT output;

	output.posModel = ip[i].posModel;
    output.normalModel = ip[i].normalModel;
    output.texcoord = ip[i].texcoord;
    output.tangentModel = ip[i].tangentModel;

	return output;
}
