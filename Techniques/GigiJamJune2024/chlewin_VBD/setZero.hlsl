// Unnamed technique, shader vbdIntegrate
/*$(ShaderResources)*/

#include "simpleRenderCommon.hlsl"

/*$(_compute:csMain)*/(uint idx : SV_DispatchThreadID)
{
	buf[idx*3] = 0;
	buf[idx*3+1] = 0;
	buf[idx*3+2] = 0;
	candidatePositions[idx] = (float4)0;
	targetPositions[idx] = (float4)0;
}
