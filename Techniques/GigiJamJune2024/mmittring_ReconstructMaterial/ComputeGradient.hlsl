// SlangAutoDiff technique, shader ComputeGradient
/*$(ShaderResources)*/

#define SLANG
#include "common.hlsl"

// https://gist.github.com/keijiro/ee7bc388272548396870
float nrand(float2 uv)
{
    return frac(sin(dot(uv, float2(12.9898, 78.233))) * 43758.5453);
}

[Differentiable]
float simpleLuminance(float3 color)
{
	return dot(color, 1 / 3.0f);
}

// @return larger means more different, 0 is equal
[Differentiable]
float ComputeColorError(float3 a, float3 b)
{
	// desaturate to make luminance more important, todo: refine
//	a = lerp(a, simpleLuminance(a), 0.75f);
//	b = lerp(b, simpleLuminance(b), 0.75f);

	float3 delta = a - b;

	return dot(delta, delta);
}

[Differentiable]
float sqr(float3 p)
{
	return dot(p, p);
}
[Differentiable]
float sqr(float4 p)
{
	return dot(p, p);
}

// todo: rename to BRDF_loss like in https://www.youtube.com/watch?v=Cpeep6R4Rmw 18:21
[Differentiable]
float ComputeImageError(no_diff float2 uv, no_diff float3 lightDirection, no_diff float3 viewDirection, /*GBuffer gbuffer,*/ float4 stateA, float4 stateB,
	no_diff float3 targetColor,
	no_diff float4 smoothStateA,
	no_diff float4 smoothStateB,
)
{
	float gBufferError = sqr(stateA - smoothStateA) + sqr(stateB - smoothStateB);

	GBuffer gbuffer;
	gbuffer.depth = 0.5f;
	gbuffer.normal = normalize(stateB.xyz * 2 - 1);

	gbuffer.baseColor = stateA.xyz;
//	gbuffer.baseColor = 0.5f;

	gbuffer.metallic = stateB.w;
	gbuffer.linearRoughness = stateA.w;

	float3 modelColor = BRDF(uv, gbuffer, lightDirection, viewDirection);

	// do not look at neighbor material attributes
	return ComputeColorError(modelColor, targetColor);
}

// @param i 0/1/2/3
float4 indexToMask(int i)
{
	float4 ret = 0;

	ret[i] = 0.1f;
	return ret;
}

//
// Hash functions by Nimitz:
// https://www.shadertoy.com/view/Xt3cDn
//

uint2 floatBitsToUint(float2 x)
{
	return asuint(x);
}

uint base_hash(uint2 p) {
    p = 1103515245U*((p >> 1U)^(p.yx));
    uint h32 = 1103515245U*((p.x)^(p.y>>3U));
    return h32^(h32 >> 16);
}

float hash1(inout float seed) {
    uint n = base_hash(asuint(float2(seed+=.1,seed+=.1)));
    return float(n)/float(0xffffffffU);
}

float2 hash2(inout float seed) {
    uint n = base_hash(asuint(float2(seed+=.1,seed+=.1)));
    uint2 rz = uint2(n, n*48271U);
    return float2(rz.xy & uint2(0x7fffffffU))/float(0x7fffffff);
}

float3 hash3(inout float seed) {
    uint n = base_hash(asuint(float2(seed+=.1,seed+=.1)));
    uint3 rz = uint3(n, n*16807U, n*48271U);
    return float3(rz & uint3(0x7fffffffU))/float(0x7fffffff);
}

float4 hash4(inout float seed) {
	return float4(hash3(seed), hash1(seed));
}

// compute blurred G-Buffer A
float4 getSmoothStateA(uint2 DTid)
{
	float4 a = saturate(StateA[DTid.xy + int2(-1, 0)]);
	float4 b = saturate(StateA[DTid.xy + int2( 1, 0)]);
	float4 c = saturate(StateA[DTid.xy + int2( 0,-1)]);
	float4 d = saturate(StateA[DTid.xy + int2( 0, 1)]);

	return (a + b + c + d) * 0.25f;
}
// compute blurred G-Buffer B
float4 getSmoothStateB(uint2 DTid)
{
	float4 a = saturate(StateB[DTid.xy + int2(-1, 0)]);
	float4 b = saturate(StateB[DTid.xy + int2( 1, 0)]);
	float4 c = saturate(StateB[DTid.xy + int2( 0,-1)]);
	float4 d = saturate(StateB[DTid.xy + int2( 0, 1)]);

	return (a + b + c + d) * 0.25f;
}

// https://www.shadertoy.com/view/7sBfDm
float  lt(float  a, float b){ return a<b?1.0:0.0;}
float  lessThan(float  a, float b){ return lt(a,b); }
float2 lessThan(float2 a, float2 b){ return float2(lt(a.x,b.x),lt(a.y,b.y) );}
float3 lessThan(float3 a, float3 b){ return float3(lt(a.x,b.x),lt(a.y,b.y),lt(a.z,b.z) );}
float4 lessThan(float4 a, float4 b){ return float4(lt(a.x,b.x),lt(a.y,b.y),lt(a.z,b.z),lt(a.w,b.w) );}

// high quality
float3 srgb_decode (float3 e) {
  return lerp(e/12.92,pow(abs(e+.055)/1.055,float3(2.4)),lessThan(float3(.04045),e));
}

// high quality
float3 srgb_encode (float3 v) {
  return lerp(12.92*v,1.055*pow(abs(v),float3(.41666))-.055,lessThan(float3(.0031308),v));
}

// https://www.shadertoy.com/view/XlycWh
float3 random_in_unit_sphere(inout float seed) {
    float3 h = hash3(seed) * float3(2.,6.28318530718,1.)-float3(1,0,0);
    float phi = h.y;
    float r = pow(h.z, 1./3.);
	return r * float3(sqrt(1.-h.x*h.x)*float2(sin(phi),cos(phi)),h.x);
}

[shader("compute")]
/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	static const uint2 c_Size = uint2/*$(Variable:Size)*/;

	// 0..1
	float2 uv = DTid.xy / (float2)c_Size;


	float4 dFLocalA = 0;
	float4 dFLocalB = 0;
	
	// G-Buffer

	// float4(albedo color 0..1, roughness 0..1)
	float4 stateA = saturate(StateA[DTid.xy]);
	// float4(normal mapped to 0..1, metallic 0..1)
	float4 stateB = saturate(StateB[DTid.xy]);

	// blurred G-Buffer
	float4 smoothStateA = getSmoothStateA(DTid.xy);
	float4 smoothStateB = getSmoothStateB(DTid.xy);

	// renormalize the normal in the G-Buffer (in 0..1 range)
	stateB.rgb = normalize(stateB.rgb * 2 - 1) * 0.5f + 0.5f;

	// up to 32 seems to make sense, smaller number seem to compile faster
	int directionIdCount = 12;

	// like in https://www.shadertoy.com/view/XlycWh
	float seed = float(base_hash(asuint((float2)DTid.xy)))/float(0xffffffffU)+/*$(Variable:FrameIndex)*/;

	[loop] for (int directionId = 0; directionId < directionIdCount; ++directionId)
	{
		// hard coded single light direction, for experiments
//		float3 lightDirection = float3(-0.6f, -0.7f, 0.7f);
		// randomize light direction
		float3 lightDirection = random_in_unit_sphere(seed);

		float3 viewDirection = random_in_unit_sphere(seed);

		GBuffer gbuffer = scene(uv);
		float3 targetColor = BRDF(uv, gbuffer, lightDirection, viewDirection);

		if (/*$(Variable:UseBackwardAD)*/ != 0)
		{
			// Backward mode automatic differentiation (AD) - AKA Backpropagation

			// get local dFdX and dFdy
			float loss = ComputeImageError(uv, lightDirection, viewDirection, stateA, stateB, targetColor, smoothStateA, smoothStateB);

			var stateAD = diffPair(stateA);
			var stateBD = diffPair(stateB);
			bwd_diff(ComputeImageError)(uv, lightDirection, viewDirection, stateAD, stateBD, targetColor, smoothStateA, smoothStateB, loss);

			// like forward
			[unroll] for (int i = 0; i < 4; ++i)
			{
				// only use if in reasonable bounds, todo: refine, maybe use isnan()

				if(stateAD.d[i] > -10 && stateAD.d[i] < 10)
					dFLocalA[i] += stateAD.d[i];
				if(stateBD.d[i] > -10 && stateBD.d[i] < 10)
					dFLocalB[i] += stateBD.d[i];
			}
		}
		else
		{
			// Forward mode automatic differentiation (AD) - AKA Dual numbers

			// works better so far

			[unroll] for (int i = 0; i < 4; ++i)
			{
				var stateAD = diffPair(stateA, indexToMask(i));
				var stateBD = diffPair(stateB, float4(0, 0, 0, 0));
				let result = fwd_diff(ComputeImageError)(uv, lightDirection, viewDirection, stateAD, stateBD, targetColor, smoothStateA, smoothStateB);

				// only use if in reasonable bounds, todo: refine, maybe use isnan()
				if(result.d > -1 && result.d < 1)
					dFLocalA[i] += result.d;
			}
			[unroll] for (int i = 0; i < 4; ++i)
			{
				var stateAD = diffPair(stateA, float4(0, 0, 0, 0));
				var stateBD = diffPair(stateB, indexToMask(i));
				let result = fwd_diff(ComputeImageError)(uv, lightDirection, viewDirection, stateAD, stateBD, targetColor, smoothStateA, smoothStateB);

				// only use if in reasonable bounds, todo: refine, maybe use isnan()
				if(result.d > -1 && result.d < 1)
					dFLocalB[i] += result.d;
			}
		}
	}

	bool reject = false;

	float lumBaseColor = simpleLuminance(stateA.xyz);
	
	GBuffer gbuffer;
	gbuffer.depth = 0.5f;
	gbuffer.normal = normalize(stateB.xyz * 2 - 1);
	gbuffer.baseColor = stateA.xyz;
	gbuffer.metallic = stateB.w;
	gbuffer.linearRoughness = stateA.w;
	OutputGradient[DTid.xy] = float4(BRDF(uv, gbuffer, float3(-0.6f, -0.7f, 0.7f), float3(0, 0, -1)), 1);

	if (/*$(Variable:Reset)*/ != 0 || reject)
	{
		seed = base_hash(DTid.xy + uint2(/*$(Variable:FrameIndex)*/ * 123, /*$(Variable:FrameIndex)*/ * 137));
		// suggested start setting from Mathieu Lamarre
		StateA[DTid.xy] = float4(0.5f, 0.5f, 0.5f, 1.0f);
		StateB[DTid.xy] = float4(0.5f, 0.5f, 1.0f, 0.0f);

		// uncomment to try with noise as start setting
//		StateA[DTid.xy] = hash4(seed);
//		StateB[DTid.xy] = hash4(seed);
	}
	else
	{
		float scale = 1.1f / 32 * 0.1f;

		StateA[DTid.xy] = stateA - scale * dFLocalA;
		StateB[DTid.xy] = stateB - scale * dFLocalB;
	}

	// uncomment to visualize colorDiff
//	OutputGradient[DTid.xy] = float4(colorDiff.xxx, 1);
}

/*
Shader Resources:
	Texture OutputGradient (as UAV)
	Texture StateA (as UAV)
	Texture StateB (as UAV)
*/
