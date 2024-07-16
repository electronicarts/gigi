// Globe technique, shader GlobeVS
/*$(ShaderResources)*/

struct VSInput
{
	uint vertexID: SV_VertexID;
	uint instanceId : SV_InstanceID;
};

struct VSOutput // AKA PSInput
{
	float4 position          : SV_POSITION;
	float2 uv                : TEXCOORD0;
	float3 worldPos          : TEXCOORD1;
	float3 normal            : TEXCOORD2;
	float4 positionThisFrame : TEXCOORD3;
	float4 positionLastFrame : TEXCOORD4;
};

static const float c_pi = 3.14159265359f;

float3 GetWorldPos(in float2 uv, in float2 scrollingUV, in float2 offset)
{
	uv += offset;
	scrollingUV += offset;

	// calculate height
	float height = /*$(Image:ldem_4_uint.exr:RGBA32_Float:float:false)*/.SampleLevel(LinearWrapVS, scrollingUV, 0).r;

	// calculate position
	float theta = uv.y * c_pi;
	float phi = uv.x * 2.0f * c_pi;

	float3 position = float3(
		sin(theta) * cos(phi),
		sin(theta) * sin(phi),
		cos(theta)
	).yzx * /*$(Variable:Scale)*/ * (1.0f + height * /*$(Variable:HeightScale)*/);

	return position;
}

float3 WorldPosOffset(int frameIndex)
{
	return float3(0.0f, 0.0f, 0.0f);

	static const int c_frameLength = 240;

	// phase is [0,1] saw wave
	float phase = frac(float(frameIndex) / float(c_frameLength));

	// x is [0,1] triangle wave
	float x;
	if (phase <= 0.5f)
		x = phase * 2.0f;
	else
		x = (1.0f - phase) * 2.0f;

	// x is [-1, 1] triangle wave
	x = x * 2.0f - 1.0f;

	// x is [-20, 20] triangle wave
	x *= 20.0f;

	return float3(x, 0.0f, 0.0f);
}

VSOutput vsmain(VSInput input)
{
	VSOutput ret = (VSOutput)0;

	// calculate UV
	float2 uv;
	{
		uint cellIndex1D = input.vertexID / 6;

		uint2 cellIndex = uint2(cellIndex1D % /*$(Variable:MeshSize1D)*/, cellIndex1D / /*$(Variable:MeshSize1D)*/);

		switch (input.vertexID % 3)
		{
			case 0: uv = float2(0.0f, 0.0f); break;
			case 1: uv = float2(1.0f, 0.0f); break;
			case 2: uv = float2(0.0f, 1.0f); break;
		}

		if (input.vertexID % 6 >= 3)
			uv = 1.0f - uv;

		uv = (uv + float2(cellIndex)) / float(/*$(Variable:MeshSize1D)*/);
	}

	// calculate position
	float2 scrollingUV = uv;
	scrollingUV.x = frac(scrollingUV.x + /*$(Variable:iFrame)*/ * /*$(Variable:RotationRadiansPerFrame)*/ / (2.0f * c_pi));
	float3 position = GetWorldPos(uv, scrollingUV, float2(0.0f, 0.0f));
	position += WorldPosOffset(/*$(Variable:iFrame)*/);

	// calculate last frame position
	float2 scrollingUVLastFrame = uv;
	scrollingUVLastFrame.x = frac(scrollingUVLastFrame.x + (/*$(Variable:iFrame)*/-1) * /*$(Variable:RotationRadiansPerFrame)*/ / (2.0f * c_pi));
	float3 positionLastFrame = GetWorldPos(uv, scrollingUVLastFrame, float2(0.0f, 0.0f));
	positionLastFrame += WorldPosOffset(/*$(Variable:iFrame)*/-1);

	// calculate normal
	float3 normal;
	{
		static const float c_cellSizeUV = 1.0f / float(/*$(Variable:MeshSize1D)*/);

		float3 positionOrigin = GetWorldPos(uv, scrollingUV, float2(0.0f, 0.0f));
		float3 positionU = GetWorldPos(uv, scrollingUV, float2(c_cellSizeUV, 0.0f));
		float3 positionV = GetWorldPos(uv, scrollingUV, float2(0.0f, c_cellSizeUV));

		float3 VU = normalize(positionU - positionOrigin);
		float3 VV = normalize(positionV - positionOrigin);
		normal = normalize(cross(VV, VU));
	}

	// send the data to the pixel shader
	ret.uv = scrollingUV;
	ret.position = mul(float4(position, 1.0f), /*$(Variable:JitteredViewProjMtx)*/);
	ret.worldPos = position;
	ret.normal = normal;
	ret.positionThisFrame = mul(float4(position, 1.0f), /*$(Variable:JitteredViewProjMtx)*/);
	ret.positionLastFrame = mul(float4(positionLastFrame, 1.0f), /*$(Variable:JitteredViewProjMtxLastFrame)*/);

	return ret;
}
