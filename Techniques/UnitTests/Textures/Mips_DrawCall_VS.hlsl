// SimpleRaster technique, shader VertexShader
/*$(ShaderResources)*/

struct VSInput
{
	float3 position   : POSITION;
	float3 normal     : NORMAL;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD1;
};

VSOutput VSMain(VSInput input)
{
	VSOutput ret = (VSOutput)0;

	float4x4 viewProjMtx = /*$(Variable:ViewProjMtx)*/;
	viewProjMtx = transpose(float4x4(
		float4(-2.412263f,  0.000000f, -0.097036f, -0.544210f),
		float4(-0.008879f,  2.404085f,  0.220737f, -1.639256f),
		float4(-0.000004f, -0.000009f,  0.000100f,  0.099687f),
		float4( 0.040025f,  0.091506f, -0.995000f,  3.225076f)
	));

	ret.position = mul(float4(input.position, 1.0f), viewProjMtx);
	ret.normal = input.normal;
	return ret;
}
