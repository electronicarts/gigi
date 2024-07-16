// simpleRaster technique, shader MeshShader
/*$(ShaderResources)*/
#include "Mesh_Shared.hlsli"

#define NUM_VERTS 126
#define NUM_TRIS NUM_VERTS / 3

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

/*$(_mesh:MSMain)*/(uint dtid : SV_DispatchThreadID, uint gtid : SV_GroupIndex, uint3 gid : SV_GroupID, out vertices ProcessedVertex verts[NUM_VERTS], out indices uint3 tris[NUM_TRIS])
{
    int numVerts = NUM_VERTS;
    int numTris = NUM_TRIS;

    SetMeshOutputCounts(numVerts, numTris);

    uint rng = wang_hash_init(gid);

    if (gtid < NUM_VERTS) 
    {
        verts[gtid].position = mul(float4(VB[dtid].Position, 1.0f), /*$(Variable:ViewProjMtx)*/);
        verts[gtid].normal = VB[dtid].Normal;

        // This shows the groups
        //verts[gtid].color = float3(wang_hash_float01(rng), wang_hash_float01(rng), wang_hash_float01(rng));

        // This shows the vertex order in the groups
        //verts[gtid].color = float3(float(gtid) / float(NUM_VERTS), 0.0f, 0.0f);

        // This shows the mesh normals
        verts[gtid].color = VB[dtid].Normal * 0.5f + 0.5f;
    }

    if (gtid < NUM_TRIS) 
    {
        tris[gtid] = uint3(gtid * 3 + 0, gtid * 3 + 1, gtid * 3 + 2);
    }
}
