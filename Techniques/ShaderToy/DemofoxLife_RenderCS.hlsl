// Unnamed technique, shader Render
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	const int simReadIndex = (/*$(Variable:FrameIndex)*/ + 1) % 2;

	int2 px = DTid.xy;

	float2 uv = float2(px) / float2(/*$(Variable:RenderSize)*/);
	int2 cellIndex = int2(uv * float2(/*$(Variable:GridSize)*/));

	uint GridValue = GridState[uint3(cellIndex, simReadIndex)];

	if (GridValue == 0)
		Color[px] = float4(0.0f, 0.0f, 0.0f, 1.0f);
	else
		Color[px] = float4(uv, 1.0f - uv.x, 1.0f);
}

/*
Shader Resources:
	Texture GridState (as SRV)
	Texture Color (as UAV)
*/
