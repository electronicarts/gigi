struct ASPayload
{
	float3 offset;
};

struct ProcessedVertex
{
	float4 position   : SV_POSITION;
	float3 normal     : TEXCOORD1;
	float3 color      : TEXCOORD2;
};

#define MESHLET_NUM_VERTS    126
#define MESHLET_NUM_TRIS     MESHLET_NUM_VERTS / 3
