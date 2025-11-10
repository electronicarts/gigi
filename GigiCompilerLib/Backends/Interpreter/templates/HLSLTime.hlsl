// abstract AMD/NVidia differences away to get time in shader code

// API:
//  uint64_t clock64();
//  uint timediff(uint startTime, uint endTime);
//  uint clock32();

// Gigi Viewer specific
#if ENV_VENDOR_NVIDIA == 1
	#define NV_API 1
#endif


#if NV_API == 1 // ------------------------------------------------

// see https://developer.nvidia.com/blog/profiling-dxr-shaders-with-timer-instrumentation/
// need R440 or later
#define NV_SHADER_EXTN_SLOT			  u0
// space4318 = can be any number but must be the same in HLSL, here: nvidia hardware vendor id 0x10de=4318
#define NV_SHADER_EXTN_REGISTER_SPACE space4318
#include "nvHLSLExtns.h" // ../Util/nv-api/nvHLSLExtns.h

uint64_t clock64()
{
	// Note: it's possible but unlikely "hi" is not fitting to "lo"
	uint hi = NvGetSpecial(NV_SPECIALOP_GLOBAL_TIMER_HI);
	uint lo = NvGetSpecial(NV_SPECIALOP_GLOBAL_TIMER_LO);

	return ((uint64_t)hi << 32) | (uint64_t)lo;
}

uint timediff(uint startTime, uint endTime)
{
	// Account for (at most one) overflow
	return endTime >= startTime ? (endTime - startTime) : (~0u - (startTime - endTime));
}

uint clock32()
{
	return NvGetSpecial(NV_SPECIALOP_GLOBAL_TIMER_LO);
}

#elif AMD_AGS == 1 // ------------------------------------------------

#include "../Util/amd-ags/ags_shader_intrinsics_dx12.hlsl"

uint64_t clock64()
{
	uint2 hilo = AmdExtD3DShaderIntrinsics_ShaderRealtimeClock();

	return ((uint64_t)hilo.y << 32) | (uint64_t)hilo.x;
}

uint timediff(uint startTime, uint endTime)
{
	// Account for (at most one) overflow
	return endTime >= startTime ? (endTime - startTime) : (~0u - (startTime - endTime));
}

uint clock32()
{
	uint2 hilo = AmdExtD3DShaderIntrinsics_ShaderRealtimeClock();
	return hilo.x;
}

#else // ------------------------------------------------

// the feature is not supported
uint64_t clock64() { return (uint64_t)0; }
uint timediff(uint startTime, uint endTime) { return 0; }
uint clock32() { return 0; }

#endif // ------------------------------------------------
