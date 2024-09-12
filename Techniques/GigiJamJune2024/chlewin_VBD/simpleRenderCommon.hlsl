
static const uint FixedPointMultiplier = 1024 * 1024;


struct VSOutput
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD0;
	float3 color 	  : TEXCOORD1;
};

struct PSOutput
{
	float4 color : SV_Target0;
};

struct VertexFormat
{
	float3 position   : POSITION;
	float3 normal     : NORMAL;
	float3 color 	  : COLOR;
};
