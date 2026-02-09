// Unnamed technique, shader ModelViewerVS
/*$(ShaderResources)*/

struct VSInput
{
	float3 Position   : POSITION;
	uint   VertexID   : SV_VertexID;
	uint   InstanceId : SV_InstanceID;
	float3 Color      : COLOR;
	float3 Normal     : NORMAL;
	float4 Tangent    : TANGENT;
	float2 UV0         : TEXCOORD0;
    float2 UV1         : TEXCOORD1;
    float2 UV2         : TEXCOORD2;
    float2 UV3         : TEXCOORD3;
	int MaterialID    : TEXCOORD4;
	int ShapeID       : TEXCOORD5;
};

struct VSOutput // AKA PSInput
{
	float4 Position   : SV_POSITION;
	float4 Color      : TEXCOORD0;
	float3 Normal     : NORMAL;
	float3 WorldPos   : POSITION;
	int MaterialID    : TEXCOORD1;
	float2 UV0	      : TEXCOORD2;
    float2 UV1	      : TEXCOORD3;
    float2 UV2	      : TEXCOORD4;
    float2 UV3	      : TEXCOORD5;
};

VSOutput vsmain(VSInput input)
{
	float4 outPos = mul(float4(input.Position, 1.0f), /*$(Variable:ViewProjMtx)*/);

	VSOutput ret = (VSOutput)0;
	switch(/*$(Variable:ViewMode)*/)
	{
		case ViewModes::InputPos: ret.Color = float4(input.Position, 1.0f); break;
		case ViewModes::OutputPos: ret.Color = float4(outPos / outPos.w); break;
		case ViewModes::VertexID: ret.Color = float4(input.VertexID, 0.0f, 0.0f, 1.0f); break;
		case ViewModes::InstanceID: ret.Color = float4(input.InstanceId, 0.0f, 0.0f, 1.0f); break;
		case ViewModes::Color: ret.Color = float4(input.Color, 1.0f); break;
		case ViewModes::Normal:
		{
			float3 displayNormal = /*$(Variable:RemapRanges)*/ ? (input.Normal + 1.0f) / 2.0f : input.Normal;
			ret.Color = float4(displayNormal, 1.0f);
			break;
		}
		case ViewModes::Tangent:
		{
			float3 displayTangent = /*$(Variable:RemapRanges)*/ ? (input.Tangent.xyz + 1.0f) / 2.0f : input.Tangent.xyz;
			ret.Color = float4(displayTangent, 1.0f);
		}
			break;
		case ViewModes::UV0: ret.Color = float4(input.UV0, 0.0f, 1.0f); break;
        case ViewModes::UV1: ret.Color = float4(input.UV1, 0.0f, 1.0f); break;
        case ViewModes::UV2: ret.Color = float4(input.UV2, 0.0f, 1.0f); break;
        case ViewModes::UV3: ret.Color = float4(input.UV3, 0.0f, 1.0f); break;
		case ViewModes::MaterialID: ret.Color = float4(input.MaterialID, 0.0f, 0.0f, 1.0f); break;
		case ViewModes::ShapeID: ret.Color = float4(input.ShapeID, 0.0f, 0.0f, 1.0f); break;
        case ViewModes::MaterialOcclusion: ret.Color = float4(0.0f, 0.0f, 0.0f, 1.0f); break;
        case ViewModes::MaterialNormal: ret.Color = float4(0.0f, 0.0f, 0.0f, 1.0f); break;
        case ViewModes::Shaded: ret.Color = float4(input.Color * /*$(Variable:ViewerColor)*/, 1.0f); break;
	}
	
	ret.Position = outPos;
	ret.WorldPos = input.Position;
	ret.Normal = input.Normal;
    ret.MaterialID = input.MaterialID;
    ret.UV0 = input.UV0;
    ret.UV1 = input.UV1;
    ret.UV2 = input.UV2;
    ret.UV3 = input.UV3;
	return ret;
}
