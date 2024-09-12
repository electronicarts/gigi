// Unnamed technique, shader calculateColors
/*$(ShaderResources)*/

static const float3 colorVisualizations[] = 
{
	float3(1,0,0),
	float3(0,1,0),
	float3(0,0,1),
	float3(1,1,0),
	float3(1,0,1),
	float3(0,1,1),
	float3(1,1,1),
	float3(0,0,0),
};

/*$(_compute:csMain)*/(uint3 DTid : SV_DispatchThreadID)
{
	vertexColors[DTid.x] = float4(1,1,1,1);
	// uint colorCount = vertexColoringData[0];
	// uint thisVertex = vertexColoringData[2 + colorCount + DTid.x];

	// uint thisColor = 0;

	// for(uint i = 0; i < colorCount; ++i)
	// {
	// 	if(vertexColoringData[2 + i] >= DTid.x)
	// 	{
	// 		thisColor = i;
	// 		break;
	// 	}
	// }

	// vertexColors[thisVertex] = float4(colorVisualizations[thisColor], 1);

}

/*
Shader Resources:
	Buffer vertexColoringData (as SRV)
*/
