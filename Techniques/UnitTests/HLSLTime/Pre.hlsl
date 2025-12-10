/*$(ShaderResources)*/

#include "HLSLTime.hlsl"		// Gigi need this registered as FileCopy with "Shader" type

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	State[0].StartTime = clock32();
}
