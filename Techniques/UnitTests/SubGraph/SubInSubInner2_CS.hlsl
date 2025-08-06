// SubInSubInner2 technique, shader Inner2CS
/*$(ShaderResources)*/

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint2 px = DTid.xy;
	/*$(RWTextureW:Output)*/[px].rgba = Input[px].gbra * /*$(Variable:Mult)*/;
}

/*
Shader Resources:
	Texture Input (as SRV)
	Texture Output (as UAV)
*/
