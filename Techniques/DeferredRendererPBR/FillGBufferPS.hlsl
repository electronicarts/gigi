// Unnamed technique, shader Fill GBuffer PS
/*$(ShaderResources)*/

#include "GBuffer.hlsl"

struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD0;
	float4 tangent    : TEXCOORD1;
	float2 uv         : TEXCOORD2;
	float3 viewPos    : TEXCOORD3;
	float3 worldPos   : TEXCOORD4;
	int    materialID : TEXCOORD6;
	int    shapeID    : TEXCOORD7;
};

struct PSOutput
{
	float4 Albedo_Roughness : SV_Target0;
	float4 Normal_Metallic_AO : SV_Target1;
	float4 Debug : SV_Target2;
};

PSOutput psmain(PSInput input)
{
	PSOutput ret = (PSOutput)0;

	// make TBN matrix via Gram-Schmidt process
	float3 T = normalize(input.tangent.rgb);
	float3 N = normalize(input.normal);
	T = normalize(T - dot(T, N) * N);
	float3 B = (input.tangent.a == 0.0f) ? cross(T, N) : cross(N, T);
	float3x3 TBN = float3x3(T, B, N);

	float3 texNormal = normalize(Normal.Sample(samplerLinearWrap, input.uv).rgb * 2.0f - 1.0f);

	float3 albedo = Albedo.Sample(samplerLinearWrap, input.uv).rgb;
	float roughness = Roughness.Sample(samplerLinearWrap, input.uv).r;
	float metallic = Metallic.Sample(samplerLinearWrap, input.uv).r;
	float ao = AO.Sample(samplerLinearWrap, input.uv).r;

	float3 normal = /*$(Variable:NormalMapping)*/ ? normalize(mul(texNormal, TBN)) : N;

	GBuffer gbuffer;
	gbuffer.normal = normal;
	gbuffer.albedo = albedo;
	gbuffer.roughness = roughness;
	gbuffer.metallic = metallic;
	gbuffer.ao = ao;

	EncodeGBuffer(gbuffer, ret.Albedo_Roughness, ret.Normal_Metallic_AO);

	switch(/*$(Variable:DebugMode)*/)
	{
		case DebugModes::None: ret.Debug = float4(0.0f, 0.0f, 0.0f, 1.0f); break;
		case DebugModes::Albedo: ret.Debug = float4(albedo.rgb, 1.0f); break;
		case DebugModes::Roughness: ret.Debug = float4(roughness, 0.0f, 0.0f, 1.0f); break;
		case DebugModes::VertexNormal: ret.Debug = float4(input.normal, 1.0f); break;
		case DebugModes::VertexTangent: ret.Debug = float4(input.tangent); break;
		case DebugModes::TBNTangent: ret.Debug = float4(T, 1.0f); break;
		case DebugModes::TBNBitangent: ret.Debug = float4(B, 1.0f); break;
		case DebugModes::TBNNormal: ret.Debug = float4(N, 1.0f); break;
		case DebugModes::ShadingNormal: ret.Debug = float4(normal, 1.0f); break;
		case DebugModes::Metallic: ret.Debug = float4(metallic, 0.0f, 0.0f, 1.0f); break;
		case DebugModes::AO: ret.Debug = float4(ao, 0.0f, 0.0f, 1.0f); break;
		case DebugModes::UV: ret.Debug = float4(input.uv, 0.0f, 1.0f); break;
		case DebugModes::Depth: ret.Debug = float4(input.position.z, 0.0f, 0.0f, 1.0f); break;
		case DebugModes::LinearDepth: ret.Debug = float4(input.viewPos.z, 0.0f, 0.0f, 1.0f); break;
		case DebugModes::WorldPos: ret.Debug = float4(input.worldPos, 1.0f); break;
		case DebugModes::ViewPos: ret.Debug = float4(input.viewPos, 1.0f); break;
		case DebugModes::MaterialID: ret.Debug = float4(input.materialID, 0.0f, 0.0f, 1.0f); break;
		case DebugModes::ShapeID: ret.Debug = float4(input.shapeID, 0.0f, 0.0f, 1.0f); break;
	}

	return ret;
}
