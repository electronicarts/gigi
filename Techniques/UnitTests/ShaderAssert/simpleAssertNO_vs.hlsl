// Unnamed technique, shader simpleAssertYES_VS
/*$(ShaderResources)*/

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
};

VSOutput main_vs()
{
	/*$(Assert: 1<2, "VS")*/

	VSOutput ret = (VSOutput)0;
	return ret;
}
