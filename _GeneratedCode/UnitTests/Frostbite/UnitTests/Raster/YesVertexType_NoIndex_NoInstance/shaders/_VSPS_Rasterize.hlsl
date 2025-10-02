//VS Source: YesVertexType_NoIndex_NoInstance_VS.hlsl
//PS Source: YesVertexType_NoIndex_NoInstance_PS.hlsl

#if FB_VS

struct Struct__VertexShaderCB
{
    float4x4 ViewProjMtx;
};

cbuffer cb0 : register(b0)
{
	Struct__VertexShaderCB _VertexShaderCB;
};


struct VSInput
{
	float3 position : POSITION;
	uint instanceId : SV_InstanceID;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
};

VSOutput VSMain(VSInput input)
{
	VSOutput ret = (VSOutput)0;

	float3 offset = float3((float(input.instanceId)-2.0f) * 2.0f, 0.0f, 0.0f);
	ret.position = mul(float4(input.position + offset, 1.0f), _VertexShaderCB.ViewProjMtx);

	return ret;
}

#endif // FB_VS

#if FB_PS



struct PSInput // AKA VSOutput
{
	float4 position   : SV_POSITION;
};

struct PSOutput
{
	float4 colorTarget : SV_Target0;
};

PSOutput PSMain(PSInput input)
{
	PSOutput ret = (PSOutput)0;
	ret.colorTarget = float4(0.0f, 1.0f, 0.0f, 1.0f);
	return ret;
}

#endif // FB_PS