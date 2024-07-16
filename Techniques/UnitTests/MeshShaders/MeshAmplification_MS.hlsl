// simpleRaster technique, shader MeshShader
/*$(ShaderResources)*/
#include "MeshAmplification_Shared.hlsli"

uint wang_hash_init(uint3 seed)
{
	return uint(seed.x * uint(1973) + seed.y * uint(9277) + seed.z * uint(26699)) | uint(1);
}

uint wang_hash_uint(inout uint seed)
{
	seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
	seed *= uint(9);
	seed = seed ^ (seed >> 4);
	seed *= uint(0x27d4eb2d);
	seed = seed ^ (seed >> 15);
	return seed;
}
float wang_hash_float01(inout uint state)
{
	return float(wang_hash_uint(state) & 0x00FFFFFF) / float(0x01000000);
}

/*$(_mesh:MSMain)*/(uint3 dtid : SV_DispatchThreadID, uint gtid : SV_GroupIndex, uint3 gid : SV_GroupID, in payload ASPayload payload, out vertices ProcessedVertex verts[MESHLET_NUM_VERTS], out indices uint3 tris[MESHLET_NUM_TRIS])
{
    int numVerts = MESHLET_NUM_VERTS;
    int numTris = MESHLET_NUM_TRIS;

    uint instanceId = dtid.z;
    float3 offset = float3(float(instanceId) * /*$(Variable:AmplificationSpacing)*/, 0.0f, 0.0f);

    SetMeshOutputCounts(numVerts, numTris);

    uint rng = wang_hash_init(gid);

    if (gtid < MESHLET_NUM_VERTS) 
    {
        verts[gtid].position = mul(float4(MeshVB[dtid.x].Position + payload.offset + offset, 1.0f), /*$(Variable:ViewProjMtx)*/);
        verts[gtid].normal = MeshVB[dtid.x].Normal;

        // This shows the groups
        //verts[gtid].color = float3(wang_hash_float01(rng), wang_hash_float01(rng), wang_hash_float01(rng));

        // This shows the vertex order in the groups
        //verts[gtid].color = float3(float(gtid) / float(MESHLET_NUM_VERTS), 0.0f, 0.0f);

        // This shows the mesh normals
        verts[gtid].color = MeshVB[dtid.x].Normal * 0.5f + 0.5f;
    }

    if (gtid < MESHLET_NUM_TRIS) 
    {
        tris[gtid] = uint3(gtid * 3 + 0, gtid * 3 + 1, gtid * 3 + 2);
    }
}
