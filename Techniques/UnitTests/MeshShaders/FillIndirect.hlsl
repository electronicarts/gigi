// Mesh technique, shader FillIndirectBuffer
/*$(ShaderResources)*/
#include "Mesh_Shared.hlsli"

uint divideAndRoundUp(uint value, uint divisor)
{
    return (value + divisor - 1) / divisor;
}

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
    uint2 vbCountStride;
    VertexBuffer.GetDimensions(vbCountStride.x, vbCountStride.y);
    Indirect[0].x = divideAndRoundUp(vbCountStride.x, NUM_VERTS);
    Indirect[0].y = 1;
    Indirect[0].z = 1;
}

/*
Shader Resources:
	Buffer VertexBuffer (as SRV)
	Buffer Indirect (as UAV)
*/
