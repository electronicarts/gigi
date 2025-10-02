// Unnamed technique, shader cs


struct Struct_BufferStruct
{
    int theInt;
    float theFloat;
};

RWTexture2D<float4> tex2D : register(u0);
RWTexture2DArray<float4> tex2DArray : register(u1);
RWTexture3D<float4> tex3D : register(u2);
RWBuffer<float> floatBuffer : register(u3);
RWStructuredBuffer<Struct_BufferStruct> structBuffer : register(u4);


[numthreads(8, 8, 1)]
void csmain(uint3 DTid : SV_DispatchThreadID)
{
}

/*
Shader Resources:
	Texture tex2D (as UAV)
	Texture tex2DArray (as UAV)
	Texture tex3D (as UAV)
	Buffer floatBuffer (as UAV)
	Buffer structBuffer (as Count)
*/
