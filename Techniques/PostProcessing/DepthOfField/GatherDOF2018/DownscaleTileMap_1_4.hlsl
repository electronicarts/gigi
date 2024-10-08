// Unnamed technique, shader DownscaleTimeMapCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;

	float CoCs[4];

	CoCs[0] = Source[px*2+uint2(0,0)];
	CoCs[1] = Source[px*2+uint2(1,0)];
	CoCs[2] = Source[px*2+uint2(0,1)];
	CoCs[3] = Source[px*2+uint2(1,1)];

	Dest[px] = max( CoCs[0], max( CoCs[1], max( CoCs[2], CoCs[3] ) ) );
}

/*
Shader Resources:
	Texture Source (as SRV)
	Texture Dest (as UAV)
*/
