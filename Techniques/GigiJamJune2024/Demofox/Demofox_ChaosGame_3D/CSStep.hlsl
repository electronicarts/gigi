// Demofox_ChaosGame technique, shader CSRender
/*$(ShaderResources)*/

#include "WangHash.hlsli"

static const float c_goldenRatioConjugate = 0.6180339887f;

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint cursor = DTid.x;
	if (cursor >= /*$(Variable:NumCursors)*/)
		return;

	uint newPointIndex = (/*$(Variable:FrameIndex)*/ >= /*$(Variable:SkipCount)*/)
		? (/*$(Variable:FrameIndex)*/ - /*$(Variable:SkipCount)*/) * /*$(Variable:NumCursors)*/ + cursor
		: 0;

	if (newPointIndex > /*$(Variable:NumPoints)*/)
		return;

	// Load state
	uint rngState = State[cursor].RngState;
	float GRState = State[cursor].GRState;
	float3 pos = State[cursor].Pos;

	// Initialize state
	if (/*$(Variable:FrameIndex)*/ == 0)
	{
		rngState = wang_hash_init(uint3(435, cursor, uint(/*$(Variable:TimeSeconds)*/*100.0f)));
		GRState = wang_hash_float01(rngState);

		uint randomVert = 0;
		switch(/*$(Variable:RNGMode)*/)
		{
			case RNGModes::UniformWhite: randomVert = wang_hash_uint(rngState) % /*$(Variable:NumVerts)*/; break;
			case RNGModes::GoldenRatio: randomVert = min(uint(GRState * float(/*$(Variable:NumVerts)*/)), /*$(Variable:NumVerts)*/ - 1); GRState = frac(GRState + c_goldenRatioConjugate); break;
		}

		switch(randomVert)
		{
			case 0: pos = /*$(Variable:Point0)*/; break;
			case 1: pos = /*$(Variable:Point1)*/; break;
			case 2: pos = /*$(Variable:Point2)*/; break;
			case 3: pos = /*$(Variable:Point3)*/; break;
			case 4: pos = /*$(Variable:Point4)*/; break;
			case 5: pos = /*$(Variable:Point5)*/; break;
			case 6: pos = /*$(Variable:Point6)*/; break;
			case 7: pos = /*$(Variable:Point7)*/; break;
			case 8: pos = /*$(Variable:Point8)*/; break;
			case 9: pos = /*$(Variable:Point9)*/; break;
		}
	}

	// choose the next vertex to head towards
	float3 target = float3(0.0f, 0.0f, 0.0f);

	uint randomVert = 0;
	switch(/*$(Variable:RNGMode)*/)
	{
		case RNGModes::UniformWhite: randomVert = wang_hash_uint(rngState) % /*$(Variable:NumVerts)*/; break;
		case RNGModes::GoldenRatio: randomVert = min(uint(GRState * float(/*$(Variable:NumVerts)*/)), /*$(Variable:NumVerts)*/ - 1); GRState = frac(GRState + c_goldenRatioConjugate); break;
	}
	switch(randomVert)
	{
		case 0: target = /*$(Variable:Point0)*/; break;
		case 1: target = /*$(Variable:Point1)*/; break;
		case 2: target = /*$(Variable:Point2)*/; break;
		case 3: target = /*$(Variable:Point3)*/; break;
		case 4: target = /*$(Variable:Point4)*/; break;
		case 5: target = /*$(Variable:Point5)*/; break;
		case 6: target = /*$(Variable:Point6)*/; break;
		case 7: target = /*$(Variable:Point7)*/; break;
		case 8: target = /*$(Variable:Point8)*/; break;
		case 9: target = /*$(Variable:Point9)*/; break;
	}

	// move towards the chosen vertex
	pos = lerp(pos, target, /*$(Variable:Fraction)*/);
	//pos = clamp(pos, 0.0f, 1.0f);

	// Don't draw the the first N points
	if (/*$(Variable:FrameIndex)*/ >= /*$(Variable:SkipCount)*/)
		Points[newPointIndex].Position = pos;

	// Save state
	State[cursor].RngState = rngState;
	State[cursor].GRState = GRState;
	State[cursor].Pos = pos;
}
