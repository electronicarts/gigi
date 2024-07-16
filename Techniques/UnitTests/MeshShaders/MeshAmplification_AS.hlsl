// simpleRaster technique, shader AmplificationShader
/*$(ShaderResources)*/
#include "MeshAmplification_Shared.hlsli"

groupshared ASPayload s_Payload;

/*$(_amplification:ASMain)*/(uint3 dtid : SV_DispatchThreadID, uint3 gtid : SV_GroupThreadID, uint3 gid : SV_GroupID)
{
    s_Payload.offset = /*$(Variable:Offset)*/;

    uint numVertices, vertexStride;
    AmpVB.GetDimensions(numVertices, vertexStride);
    uint dispatchX = ((numVertices + MESHLET_NUM_VERTS - 1) / MESHLET_NUM_VERTS); // divide but round up

    DispatchMesh(dispatchX, 1, /*$(Variable:AmplificationCount)*/, s_Payload);
}
