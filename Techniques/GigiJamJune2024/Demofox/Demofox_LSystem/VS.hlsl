// Unnamed technique, shader VS
/*$(ShaderResources)*/

struct VSInput
{
	float3 position   : POSITION;
	float4 color      : COLOR0;
	uint   id         : SV_VertexID;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float4 color      : TEXCOORD0;
};

VSOutput main(VSInput input)
{
	#if SECOND_EYE == 1
		float3 offset = /*$(Variable:InvViewMtx)*/[0].xyz * /*$(Variable:RedBlue3DEyeDistance)*/;
	#else
		float3 offset = float3(0.0f, 0.0f, 0.0f);	
	#endif

	VSOutput ret = (VSOutput)0;

	float3 position = (input.id < GlobalState[0].LineListCount) ? input.position : float3(0.0f, 0.0f, 0.0f);

	bool maxSymbolsHit = GlobalState[0].SymbolsCount >= /*$(Variable:SymbolsSizeMax)*/;
	bool maxVertsHit = GlobalState[0].LineListCount >= /*$(Variable:LinesMax)*/;
	bool cursorStateStackOverflow = GlobalState[0].CursorStateStackOverflow;

	float3 worldPos = position;

	if (/*$(Variable:ForceBoundingBox)*/)
	{
		float3 LinesBB = (GlobalState[0].LinesBBMax - GlobalState[0].LinesBBMin);
		float3 LinesMid = (GlobalState[0].LinesBBMax + GlobalState[0].LinesBBMin) / 2.0f;

		float LinesBBMax = max(LinesBB.x, max(LinesBB.y, LinesBB.z));

		float targetMid = (/*$(Variable:BBMax)*/ + /*$(Variable:BBMin)*/) / 2.0f;
		float targetBBMax = (/*$(Variable:BBMax)*/ - /*$(Variable:BBMin)*/);

		float scale = targetBBMax / LinesBBMax;

		worldPos = (worldPos - LinesMid) * scale;
	}

	ret.position = mul(float4(worldPos + offset, 1.0f), /*$(Variable:ViewProjMtx)*/);
	ret.color = input.color;

	if (maxSymbolsHit || maxVertsHit || cursorStateStackOverflow)
		ret.color = float4(maxSymbolsHit ? 0.5f : 0.0f, cursorStateStackOverflow ? 0.5f : 0.0f, maxVertsHit ? 0.5f : 0.0f, 1.0f);

	return ret;
}
