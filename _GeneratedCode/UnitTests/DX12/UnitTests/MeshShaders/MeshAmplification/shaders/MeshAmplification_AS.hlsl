// simpleRaster technique, shader AmplificationShader


struct Struct_VertexFormat
{
    float3 Position;
    float3 Normal;
};

struct Struct__AmplificationShaderCB
{
    uint AmplificationCount;
    float3 Offset;
};

StructuredBuffer<Struct_VertexFormat> AmpVB : register(t0);
ConstantBuffer<Struct__AmplificationShaderCB> _AmplificationShaderCB : register(b0);

#line 2

#include "MeshAmplification_Shared.hlsli"

groupshared ASPayload s_Payload;

[numthreads(4, 4, 4)]
#line 7
void ASMain(uint3 dtid : SV_DispatchThreadID, uint3 gtid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    s_Payload.offset = _AmplificationShaderCB.Offset;

    uint numVertices, vertexStride;
    AmpVB.GetDimensions(numVertices, vertexStride);
    uint dispatchX = ((numVertices + MESHLET_NUM_VERTS - 1) / MESHLET_NUM_VERTS); // divide but round up

    DispatchMesh(dispatchX, 1, _AmplificationShaderCB.AmplificationCount, s_Payload);
}
