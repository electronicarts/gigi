// Unnamed technique, shader simpleAssertYES_PS
/*$(ShaderResources)*/


#line 2


struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
	//TODO: fill this out
};

struct PSOutput
{
	float4 colorTarget : SV_Target0;
	//TODO: fill this out
};

PSOutput main_ps(PSInput input)
{
	/*$(Assert: 2>5, "PS" )*/



	PSOutput ret = (PSOutput)0;
	// TODO: fill this out
	return ret;
}
