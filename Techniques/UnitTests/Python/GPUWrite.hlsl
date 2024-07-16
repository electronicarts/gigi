// Unnamed technique, shader cs
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
}

/*
Shader Resources:
	Texture tex2D (as UAV)
	Texture tex2DArray (as UAV)
	Texture tex3D (as UAV)
	Buffer floatBuffer (as UAV)
	Buffer structBuffer (as Count)
*/
