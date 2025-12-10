// Unnamed technique, shader VS
/*$(ShaderResources)*/

struct VSInput
{
	float3 position   : POSITION;
	float3 normal     : NORMAL;
	float2 UV         : TEXCOORD0;
	int    materialID : TEXCOORD1;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float2 UV         : TEXCOORD0;
	float3 normal     : TEXCOORD1;
	int    materialID : TEXCOORD2;
	float  depth      : TEXCOORD3;
	#if OPAQUE_PASS == 1
		float4 positionLastFrame : TEXCOORD4;
		float4 positionThisFrame : TEXCOORD5;
	#endif	
};

VSOutput vsmain(VSInput input)
{
    float3 meshPos = input.position * /*$(Variable:MeshScale)*/;

    VSOutput ret = (VSOutput)0;
    ret.position = mul(float4(meshPos, 1.0f), /*$(Variable:JitteredViewProjMtx)*/);
	ret.UV = input.UV;
	ret.normal = input.normal;
	ret.materialID = input.materialID;

    float4 cameraPos = mul(float4(meshPos, 1.0f), /*$(Variable:ViewMtx)*/);
	ret.depth = cameraPos.z / cameraPos.w;

	#if OPAQUE_PASS == 1
		ret.positionThisFrame = mul(float4(meshPos, 1.0f), /*$(Variable:ViewProjMtx)*/);
		ret.positionLastFrame = mul(float4(meshPos, 1.0f), /*$(Variable:ViewProjMtxLastFrame)*/);
	#endif

	return ret;
}
