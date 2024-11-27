// Unnamed technique, shader ModelViewerCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	uint vertexID = DTid.xy;
	Struct_VB vertex = VertexBuffer[vertexID];

	// TODO: you can modify the vertex data.
	//vertex.Position *= ((sin(vertex.Position.x) * 0.5f + 0.5f) + 0.5f);

	VertexBuffer[vertexID] = vertex;
}

/*
Shader Resources:
	Buffer VertexBuffer (as UAV)
*/
