// simpleRT technique, shader SimpleRTModifyScene
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	Struct_VertexBuffer vertex = Scene[DTid.x];

	vertex.Position *= (1.0f + sin(6.28 * float(/*$(Variable:FrameIndex)*/) / 60.0f) * 0.5f + 0.5f);

	Scene[DTid.x] = vertex;
}

/*
Shader Resources:
	Buffer Scene (as UAV)
*/
