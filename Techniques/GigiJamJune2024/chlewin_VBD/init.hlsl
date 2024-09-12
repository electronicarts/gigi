// Unnamed technique, shader vbdIntegrate
/*$(ShaderResources)*/

#include "simpleRenderCommon.hlsl"

/*$(_compute:csMain)*/(uint idx : SV_DispatchThreadID)
{
	vertexVelocities[idx] = (float4)0;
	vertexPrevVelocities[idx] = (float4)0;
}
