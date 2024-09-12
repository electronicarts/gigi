// Demofox_ChaosGame technique, shader CSInit
/*$(ShaderResources)*/

#include "WangHash.hlsli"
#include "SRGB.hlsli"
#include "IndexToColor.hlsli"

float3 DrawCircle(float2 p, float3 pixelColor, float2 circlePos, float circleRadiusInner, float circleRadiusOuter, float3 circleColor)
{
	float dist = length(p - circlePos);
	float alpha = smoothstep(circleRadiusInner, circleRadiusOuter, dist);
	alpha = 1.0f - alpha;
	return lerp(pixelColor, circleColor, alpha);
}

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	// Set a default background color
	uint2 px = DTid.xy;	
	float3 pixelColor = float3(0.2f, 0.2f, 0.2f);

	// Draw the vertices
	float2 uv = float2(px) / float2(/*$(Variable:RenderSize)*/);
	uv.y = 1.0f - uv.y;

	static const float c_circleSizeInner = 0.005f;
	static const float c_circleSizeOuter = c_circleSizeInner + 2.0f / float(min(/*$(Variable:RenderSize)*/.x, /*$(Variable:RenderSize)*/.y));

	if (/*$(Variable:NumVerts)*/ > 0)
		pixelColor = DrawCircle(uv, pixelColor, /*$(Variable:Point0)*/, c_circleSizeInner, c_circleSizeOuter, SRGBToLinear(IndexToColor(0)));
	if (/*$(Variable:NumVerts)*/ > 1)
		pixelColor = DrawCircle(uv, pixelColor, /*$(Variable:Point1)*/, c_circleSizeInner, c_circleSizeOuter, SRGBToLinear(IndexToColor(1)));
	if (/*$(Variable:NumVerts)*/ > 2)
		pixelColor = DrawCircle(uv, pixelColor, /*$(Variable:Point2)*/, c_circleSizeInner, c_circleSizeOuter, SRGBToLinear(IndexToColor(2)));
	if (/*$(Variable:NumVerts)*/ > 3)
		pixelColor = DrawCircle(uv, pixelColor, /*$(Variable:Point3)*/, c_circleSizeInner, c_circleSizeOuter, SRGBToLinear(IndexToColor(3)));
	if (/*$(Variable:NumVerts)*/ > 4)
		pixelColor = DrawCircle(uv, pixelColor, /*$(Variable:Point4)*/, c_circleSizeInner, c_circleSizeOuter, SRGBToLinear(IndexToColor(4)));
	if (/*$(Variable:NumVerts)*/ > 5)
		pixelColor = DrawCircle(uv, pixelColor, /*$(Variable:Point5)*/, c_circleSizeInner, c_circleSizeOuter, SRGBToLinear(IndexToColor(5)));
	if (/*$(Variable:NumVerts)*/ > 6)
		pixelColor = DrawCircle(uv, pixelColor, /*$(Variable:Point6)*/, c_circleSizeInner, c_circleSizeOuter, SRGBToLinear(IndexToColor(6)));
	if (/*$(Variable:NumVerts)*/ > 7)
		pixelColor = DrawCircle(uv, pixelColor, /*$(Variable:Point7)*/, c_circleSizeInner, c_circleSizeOuter, SRGBToLinear(IndexToColor(7)));
	if (/*$(Variable:NumVerts)*/ > 8)
		pixelColor = DrawCircle(uv, pixelColor, /*$(Variable:Point8)*/, c_circleSizeInner, c_circleSizeOuter, SRGBToLinear(IndexToColor(8)));
	if (/*$(Variable:NumVerts)*/ > 9)
		pixelColor = DrawCircle(uv, pixelColor, /*$(Variable:Point9)*/, c_circleSizeInner, c_circleSizeOuter, SRGBToLinear(IndexToColor(9)));

	// write out the final color
	Screen[px] = float4(pixelColor, 1.0f);	
}
