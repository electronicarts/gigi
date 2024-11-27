// Unnamed technique, shader TextureViewerCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;
	float4 color = texture[px];

	// TODO: you can modify the color here

	texture[px] = color;
}

/*
Shader Resources:
	Texture texture (as UAV)
*/
