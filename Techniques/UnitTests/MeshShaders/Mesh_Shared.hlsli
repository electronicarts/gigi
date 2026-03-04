struct ProcessedVertex
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD1;
	float3 color      : TEXCOORD2;
};

// Mesh shader settings
#define NUM_VERTS 126
#define NUM_TRIS NUM_VERTS / 3
