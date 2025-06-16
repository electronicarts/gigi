// Barrier technique, shader Right
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 dims;
	Output.GetDimensions(dims.x, dims.y);
	/*$(RWTextureW:Output)*/[DTid.xy + uint2(dims.x / 2, 0)] = float4(0.0f, 0.5f, 0.0f, 1.0f);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
