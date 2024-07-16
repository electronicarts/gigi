// DemofoxNeonDesert technique, shader Bloom Horizontal
/*$(ShaderResources)*/

#include "DemofoxNeonDesert_common.hlsl"

/*$(_compute:bloomh)*/(uint3 DTid : SV_DispatchThreadID)
{
	scratch[DTid.xy] = float4(BloomPass1(float2(DTid.xy), accumulation), 1.0f);
}
