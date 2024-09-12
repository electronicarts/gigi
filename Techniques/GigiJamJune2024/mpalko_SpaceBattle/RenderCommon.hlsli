#ifndef RENDER_COMMON_HLSL
#define RENDER_COMMON_HLSL

#ifdef SHADER_MS
#define EntityStates EntityStatesMS
#endif

#ifdef SHADER_PS
#define EntityStates EntityStatesPS
#endif

struct VertexData
{
	float4 position : SV_POSITION;
	float2 uv: TEXCOORD0;
	float3 barycentric: TEXCOORD1;
	uint entityidx : TEXCOORD2;
	uint meshPart : TEXCOORD3;
};

#endif // #ifndef RENDER_COMMON_HLSL