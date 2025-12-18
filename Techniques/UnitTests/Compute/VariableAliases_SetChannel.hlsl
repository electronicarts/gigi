// Unnamed technique, shader SetChannel
/*$(ShaderResources)*/

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	// Get the pixel color
	uint2 px = DTid.xy;
	float3 pixelValue = /*$(RWTextureR:Output)*/[px].rgb;

    // Set the specified channel to the specified value
    pixelValue[/*$(VariableAlias:Channel)*/] = /*$(VariableAlias:Value)*/ * /*$(Variable:ConstMultiplier)*/;

	// Write it back
	Output[px] = float4(pixelValue, 1.0);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
