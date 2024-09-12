// Demofox_ChaosGame_3D technique, shader VSDraw
/*$(ShaderResources)*/

struct VSInput
{
	float3 position   : POSITION;
};

struct VSOutput // AKA PSInput
{
	float4 position   : SV_POSITION;
	float3 color      : TEXCOORD0;
};

VSOutput main(VSInput input)
{
	#if SECOND_EYE == 1
		float3 offset = /*$(Variable:InvViewMtx)*/[0].xyz * /*$(Variable:RedBlue3DEyeDistance)*/;
	#else
		float3 offset = float3(0.0f, 0.0f, 0.0f);	
	#endif

	VSOutput ret = (VSOutput)0;
	ret.position = mul(float4(input.position + offset, 1.0f), /*$(Variable:ViewProjMtx)*/);

// In red/blue 3d mode, the varying colors of the normal color path make the result better
#if 0	
	if (/*$(Variable:RedBlue3D)*/)
	{
		ret.color = float3(0.8f, 0.8f, 0.8f);
	}
	else
#endif
	{
		float3 BBMin01 = min(/*$(Variable:Point0)*/, /*$(Variable:Point1)*/);
		float3 BBMin23 = min(/*$(Variable:Point2)*/, /*$(Variable:Point3)*/);
		float3 BBMin45 = min(/*$(Variable:Point4)*/, /*$(Variable:Point5)*/);
		float3 BBMin67 = min(/*$(Variable:Point6)*/, /*$(Variable:Point7)*/);
		float3 BBMin89 = min(/*$(Variable:Point8)*/, /*$(Variable:Point9)*/);
		float3 BBMin0123 = min(BBMin01, BBMin23);
		float3 BBMin4567 = min(BBMin45, BBMin67);
		float3 BBMin01234567 = min(BBMin0123, BBMin4567);
		float3 BBMin = min(BBMin01234567, BBMin89);

		float3 BBMax01 = max(/*$(Variable:Point0)*/, /*$(Variable:Point1)*/);
		float3 BBMax23 = max(/*$(Variable:Point2)*/, /*$(Variable:Point3)*/);
		float3 BBMax45 = max(/*$(Variable:Point4)*/, /*$(Variable:Point5)*/);
		float3 BBMax67 = max(/*$(Variable:Point6)*/, /*$(Variable:Point7)*/);
		float3 BBMax89 = max(/*$(Variable:Point8)*/, /*$(Variable:Point9)*/);
		float3 BBMax0123 = max(BBMax01, BBMax23);
		float3 BBMax4567 = max(BBMax45, BBMax67);
		float3 BBMax01234567 = max(BBMax0123, BBMax4567);
		float3 BBMax = max(BBMax01234567, BBMax89);

		ret.color = (input.position - BBMin) / (BBMax - BBMin);
	}

	return ret;
}
