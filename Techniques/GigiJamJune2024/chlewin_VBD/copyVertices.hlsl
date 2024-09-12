/*$(ShaderResources)*/

#include "simpleRenderCommon.hlsl"

/*$(_compute:csMain)*/(uint DispatchThreadId : SV_DispatchThreadID)
{
	Struct_VertexFormat vertex;

	vertex.position = vertexPositions[DispatchThreadId].xyz;

	float3 normal = float3
	(
		float(vertexNormals[DispatchThreadId*3]) / FixedPointMultiplier,
		float(vertexNormals[DispatchThreadId*3 + 1]) / FixedPointMultiplier,
		float(vertexNormals[DispatchThreadId*3 + 2]) / FixedPointMultiplier
	);


	vertex.normal = normalize(normal);
	vertex.color = vertexColors[DispatchThreadId];

	vertices[DispatchThreadId] = vertex;
}