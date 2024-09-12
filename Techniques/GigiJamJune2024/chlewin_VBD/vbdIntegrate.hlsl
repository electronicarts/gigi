// Unnamed technique, shader vbdIntegrate
/*$(ShaderResources)*/

/*$(_compute:csMain)*/(uint idx : SV_DispatchThreadID)
{
	vertexVelocities[idx] = 60 * (vertexPositions[idx] - vertexFrameStartPositions[idx]);
}
