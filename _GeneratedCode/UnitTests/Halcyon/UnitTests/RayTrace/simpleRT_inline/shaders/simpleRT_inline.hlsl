

struct Struct_VertexBuffer
{
    float3 Color;
    float3 Position;
    float3 Normal;
};

struct Struct__SimpleRTCS_0CB
{
    float3 cameraPos;
    float _padding0;
    float4x4 clipToWorld;
    float depthNearPlane;
    float3 hitColor;
    float3 missColor;
    float _padding1;
};

RWTexture2D<float4> g_texture : register(u0);
RaytracingAccelerationStructure g_scene : register(t0);
StructuredBuffer<Struct_VertexBuffer> g_vertexBuffer : register(t1);
ConstantBuffer<Struct__SimpleRTCS_0CB> _SimpleRTCS_0CB : register(b0);


float3 LinearToSRGB(float3 linearCol)
{
	float3 sRGBLo = linearCol * 12.92;
	float3 sRGBHi = (pow(abs(linearCol), float3(1.0 / 2.4, 1.0 / 2.4, 1.0 / 2.4)) * 1.055) - 0.055;
	float3 sRGB;
	sRGB.r = linearCol.r <= 0.0031308 ? sRGBLo.r : sRGBHi.r;
	sRGB.g = linearCol.g <= 0.0031308 ? sRGBLo.g : sRGBHi.g;
	sRGB.b = linearCol.b <= 0.0031308 ? sRGBLo.b : sRGBHi.b;
	return sRGB;
}

[numthreads(8, 8, 1)]
void SimpleRTCS(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;
	uint w, h;
	g_texture.GetDimensions(w, h);
	float2 dimensions = float2(w, h);

	float2 screenPos = (float2(px)+0.5f) / dimensions * 2.0 - 1.0;
	screenPos.y = -screenPos.y;

	float4 world = mul(float4(screenPos, _SimpleRTCS_0CB.depthNearPlane, 1), _SimpleRTCS_0CB.clipToWorld);
	world.xyz /= world.w;

	RayDesc ray;
	ray.Origin = _SimpleRTCS_0CB.cameraPos;
	ray.TMin = 0;
	ray.TMax = 10000.0f;
	ray.Direction = normalize(world.xyz - ray.Origin);

	RayQuery<RAY_FLAG_CULL_NON_OPAQUE |
		RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES |
		RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> rayQuery;

	rayQuery.TraceRayInline(
		g_scene,
		0,
		255,
		ray
	);

	rayQuery.Proceed();

	if (rayQuery.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
	{
		g_texture[px] = float4(LinearToSRGB(g_vertexBuffer[rayQuery.CandidatePrimitiveIndex()*3].Normal*0.5f+0.5f), 1.0f);
		//g_texture[px] = float4(_SimpleRTCS_0CB.hitColor, 1.0f);
	}
	else
		g_texture[px] = float4(LinearToSRGB(_SimpleRTCS_0CB.missColor), 1.0f);
}
