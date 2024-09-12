/*$(ShaderResources)*/

#include "simpleRenderCommon.hlsl"

VSOutput vsMain(VertexFormat input)
{
	VSOutput ret = (VSOutput)0;
	float4x4 vpm = /*$(Variable:ViewProjMtx)*/;
	ret.position = mul(float4(input.position, 1.0f), vpm);
	ret.normal = input.normal;
	ret.color = input.color;

	return ret;
}
