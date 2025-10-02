// SlangAutoDiff technique, shader Init


struct Struct__Init_0CB
{
    int FrameIndex;
    float3 _padding0;
    float4 MouseState;
    float4 MouseStateLastFrame;
    int NumGaussians;
    float3 iResolution;
    uint initialized;
    float3 _padding1;
};

RWBuffer<float> Data : register(u0);
ConstantBuffer<Struct__Init_0CB> _Init_0CB : register(b0);


static const float c_pi = 3.14159265359f;

uint wang_hash_init(uint3 seed)
{
	return uint(seed.x * uint(1973) + seed.y * uint(9277) + seed.z * uint(26699)) | uint(1);
}

uint wang_hash_uint(inout uint seed)
{
	seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
	seed *= uint(9);
	seed = seed ^ (seed >> 4);
	seed *= uint(0x27d4eb2d);
	seed = seed ^ (seed >> 15);
	return seed;
}

float wang_hash_float01(inout uint state)
{
	return float(wang_hash_uint(state) & 0x00FFFFFF) / float(0x01000000);
}

[numthreads(1, 1, 1)]
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	// set the ball position on click
	float4 mouseState = _Init_0CB.MouseState;
	float4 mouseStateLastFrame = _Init_0CB.MouseStateLastFrame;
	if (mouseState.z && !mouseStateLastFrame.z)
	{
		float2 uv = mouseState.xy / _Init_0CB.iResolution.xy;
		Data[0] = uv.x;
		Data[1] = uv.y;
	}

	if((bool)_Init_0CB.initialized)
	{
		return;
	}

	uint rng = wang_hash_init(uint3(0x1337b4b3, 0xcafef00d, _Init_0CB.FrameIndex));
	
	// put the ball in the center
	Data[0] = 0.5f;
	Data[1] = 0.5f;

	// initialize gradient to 0
	Data[2] = 0.0f;
	Data[3] = 0.0f;

	for (int i = 0; i < _Init_0CB.NumGaussians; ++i)
	{
		// position
		Data[4+i*5+0] = wang_hash_float01(rng);
		Data[4+i*5+1] = wang_hash_float01(rng);

		// angle
		Data[4+i*5+2] = wang_hash_float01(rng) * 2.0f * c_pi;

		// sigma
		Data[4+i*5+3] = lerp(0.05f, 0.3f, wang_hash_float01(rng));
		Data[4+i*5+4] = lerp(0.05f, 0.3f, wang_hash_float01(rng));
	}
}

/*
Shader Resources:
	Buffer Data (as UAV)
*/
