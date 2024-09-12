// Unnamed technique, shader BBVS
/*$(ShaderResources)*/

struct VSInput
{
	uint   id         : SV_VertexID;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float3 color      : TEXCOORD0;
};

VSOutput main(VSInput input)
{
	VSOutput ret = (VSOutput)0;

	//float3 BBMin = GlobalState[0].LinesBBMin;
	//float3 BBMax = GlobalState[0].LinesBBMax;

	float3 cursorPos = GlobalState[0].CursorEndPosition;
	float3 cursorXAxis = GlobalState[0].CursorEndXAxis;
	float3 cursorYAxis = GlobalState[0].CursorEndYAxis;
	float3 cursorZAxis = GlobalState[0].CursorEndZAxis;

	float3 BBMin = /*$(Variable:BBMin)*/;
	float3 BBMax = /*$(Variable:BBMax)*/;

	// 12 lines total = 24 vertices
	float3 color = float3(1.0f, 1.0f, 0.0f);
	float3 worldPos = float3(0.0f, 0.0f, 0.0f);
	switch(input.id)
	{
		// Bottom square
		case 0: worldPos = float3(0.0f, 0.0f, 0.0f); break;
		case 1: worldPos = float3(1.0f, 0.0f, 0.0f); break;

		case 2: worldPos = float3(1.0f, 0.0f, 0.0f); break;
		case 3: worldPos = float3(1.0f, 0.0f, 1.0f); break;

		case 4: worldPos = float3(1.0f, 0.0f, 1.0f); break;
		case 5: worldPos = float3(0.0f, 0.0f, 1.0f); break;

		case 6: worldPos = float3(0.0f, 0.0f, 1.0f); break;
		case 7: worldPos = float3(0.0f, 0.0f, 0.0f); break;

		// Top square
		case 8: worldPos = float3(0.0f, 1.0f, 0.0f); break;
		case 9: worldPos = float3(1.0f, 1.0f, 0.0f); break;

		case 10: worldPos = float3(1.0f, 1.0f, 0.0f); break;
		case 11: worldPos = float3(1.0f, 1.0f, 1.0f); break;

		case 12: worldPos = float3(1.0f, 1.0f, 1.0f); break;
		case 13: worldPos = float3(0.0f, 1.0f, 1.0f); break;

		case 14: worldPos = float3(0.0f, 1.0f, 1.0f); break;
		case 15: worldPos = float3(0.0f, 1.0f, 0.0f); break;

		// Vertical lines
		case 16: worldPos = float3(0.0f, 0.0f, 0.0f); break;
		case 17: worldPos = float3(0.0f, 1.0f, 0.0f); break;

		case 18: worldPos = float3(1.0f, 0.0f, 0.0f); break;
		case 19: worldPos = float3(1.0f, 1.0f, 0.0f); break;

		case 20: worldPos = float3(1.0f, 0.0f, 1.0f); break;
		case 21: worldPos = float3(1.0f, 1.0f, 1.0f); break;

		case 22: worldPos = float3(0.0f, 0.0f, 1.0f); break;
		case 23: worldPos = float3(0.0f, 1.0f, 1.0f); break;

		// Cursor axis
		case 24: worldPos = cursorPos; color = float3(1.0f, 0.0f, 0.0f); break;
		case 25: worldPos = cursorPos + cursorXAxis; color = float3(1.0f, 0.0f, 0.0f); break;

		case 26: worldPos = cursorPos; color = float3(0.0f, 1.0f, 0.0f); break;
		case 27: worldPos = cursorPos + cursorYAxis; color = float3(0.0f, 1.0f, 0.0f); break;

		case 28: worldPos = cursorPos; color = float3(0.0f, 0.0f, 1.0f); break;
		case 29: worldPos = cursorPos + cursorZAxis; color = float3(0.0f, 0.0f, 1.0f); break;
	}

	if (input.id >= 24)
	{
		if(/*$(Variable:ForceBoundingBox)*/)
		{
			float3 LinesBB = (GlobalState[0].LinesBBMax - GlobalState[0].LinesBBMin);
			float3 LinesMid = (GlobalState[0].LinesBBMax + GlobalState[0].LinesBBMin) / 2.0f;

			float LinesBBMax = max(LinesBB.x, max(LinesBB.y, LinesBB.z));

			float targetMid = (/*$(Variable:BBMax)*/ + /*$(Variable:BBMin)*/) / 2.0f;
			float targetBBMax = (/*$(Variable:BBMax)*/ - /*$(Variable:BBMin)*/);

			float scale = targetBBMax / LinesBBMax;

			worldPos = (worldPos - LinesMid) * scale;
		}
	}
	else
	{
		worldPos = lerp(BBMin, BBMax, worldPos);
	}

	ret.position = mul(float4(worldPos, 1.0f), /*$(Variable:ViewProjMtx)*/);
	ret.color = color;

	return ret;
}

/*
Shader Resources:
	Buffer GlobalState (as SRV)
*/
