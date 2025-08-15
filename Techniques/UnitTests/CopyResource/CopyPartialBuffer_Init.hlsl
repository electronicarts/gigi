// Unnamed technique, shader FillInput
/*$(ShaderResources)*/

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
	Input[DTid.x] = float(DTid.x);
	Output[DTid.x] = 0.0f;
}

/*
Shader Resources:
	Buffer Input (as UAV)
	Buffer Output (as UAV)
*/
