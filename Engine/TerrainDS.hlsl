#include "Common.hlsli"

Texture2D g_heightTexture : register(t0);

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


[domain("quad")]
PixelShaderInput main(
	HS_CONSTANT_DATA_OUTPUT patchConst,
	float2 uv : SV_DomainLocation,
	const OutputPatch<HS_CONTROL_POINT_OUTPUT, NUM_CONTROL_POINTS> patch)
{
    PixelShaderInput output;

	// Bilinear interpolation.
    float3 v1 = lerp(patch[0].posModel, patch[1].posModel, uv.x);
    float3 v2 = lerp(patch[2].posModel, patch[3].posModel, uv.x);
    output.posModel = lerp(v1, v2, uv.y);
    output.posWorld = mul(float4(output.posModel, 1.0), world).xyz;

    float3 n1 = lerp(patch[0].normalModel, patch[1].normalModel, uv.x);
    float3 n2 = lerp(patch[2].normalModel, patch[3].normalModel, uv.x);
    float3 n = lerp(n1, n2, uv.y);
    output.normalWorld = mul(float4(n, 0.0), worldIT).xyz;
    output.normalWorld = normalize(output.normalWorld);
    
    float2 tex1 = lerp(patch[0].texcoord, patch[1].texcoord, uv.x);
    float2 tex2 = lerp(patch[2].texcoord, patch[3].texcoord, uv.x);
    output.texcoord = lerp(tex1, tex2, uv.y);
        
    if (useHeightMap)
    {
        float height = g_heightTexture.SampleLevel(linearWrapSampler, output.texcoord, 0).r;
        height = height * 2.0 - 1.0;
        output.posWorld += output.normalWorld * height * heightScale;
    }
    output.posProj = mul(float4(output.posWorld, 1.0), viewProj);

    
    // tangentWorld юс╫ц
    float3 tan1 = lerp(patch[0].tangentModel, patch[1].tangentModel, uv.x);
    float3 tan2 = lerp(patch[2].tangentModel, patch[3].tangentModel, uv.x);
    float3 tan = lerp(tan1, tan2, uv.y);

    output.tangentWorld = mul(float4(tan, 0.0f), world).xyz;
    
	return output;
}
