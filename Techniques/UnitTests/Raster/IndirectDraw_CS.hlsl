/*$(ShaderResources)*/

[numthreads(1,1,1)]
void CSMain(uint dtid : SV_DispatchThreadID)
{
	if (dtid == 0)
	{
		IndirectArgsBuf[0] = 3*/*$(Variable:NumIndirectTriangles)*/;
		IndirectArgsBuf[1] = 1;
		IndirectArgsBuf[2] = 0;
		IndirectArgsBuf[3] = 0;
		IndirectArgsBuf[4] = 0;

		for (int i = 0; i < 3*/*$(Variable:NumIndirectTriangles)*/; ++i)
		{
			IndirectIndexBuf[3*i + 0] = 3*i + 0;
			IndirectIndexBuf[3*i + 1] = 3*i + 1;
			IndirectIndexBuf[3*i + 2] = 3*i + 2;

			IndirectVertexBuf[3*i + 0] = float2(0, 0);
			IndirectVertexBuf[3*i + 1] = float2(0.1, 0);
			IndirectVertexBuf[3*i + 2] = float2(0, 0.1);
		}
	}
}
