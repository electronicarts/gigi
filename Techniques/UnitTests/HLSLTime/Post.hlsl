/*$(ShaderResources)*/

#include "HLSLTime.hlsl"		// Gigi need this registered as FileCopy with "Shader" type

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	// TEST = 0/1/2
	State[TEST].EndTime = clock32();
}
