/*$(ShaderResources)*/

#include "simpleRenderCommon.hlsl"

void addVertexNormal(uint index, float3 normal)
{
	uint3 n = int3(FixedPointMultiplier * normal);
	InterlockedAdd(vertexNormals[index * 3], n.x);
	InterlockedAdd(vertexNormals[index * 3 + 1], n.y);
	InterlockedAdd(vertexNormals[index * 3 + 2], n.z);

}

/*$(_compute:csMain)*/(uint DispatchThreadId : SV_DispatchThreadID)
{
	uint i0 = triangleIndices[DispatchThreadId * 3];
	uint i1 = triangleIndices[DispatchThreadId * 3 + 1];
	uint i2 = triangleIndices[DispatchThreadId * 3 + 2];

	float3 p0 = vertexPositions[i0].xyz;
	float3 p1 = vertexPositions[i1].xyz;
	float3 p2 = vertexPositions[i2].xyz;

	float3 n = normalize(cross(p2 - p0, p1 - p0));
	
	addVertexNormal(i0, n);
	addVertexNormal(i1, n);
	addVertexNormal(i2, n);
}