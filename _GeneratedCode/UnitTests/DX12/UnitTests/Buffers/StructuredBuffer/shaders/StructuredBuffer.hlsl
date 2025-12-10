// BufferFormats technique, shader csmain


struct Trinary
{
    static const int True = 0;
    static const int False = 1;
    static const int Maybe = 2;
};

struct Struct_TheStructure
{
    int TheInt;
    float TheFloat;
    int TheEnum;
    uint TheBool;
    uint TheUINT;
    float2 TheFloat2;
};

struct Struct__csmain_0CB
{
    float frameDeltaTime;
    int frameIndex;
    float2 _padding0;
};

RWStructuredBuffer<Struct_TheStructure> buff : register(u0);
ConstantBuffer<Struct__csmain_0CB> _csmain_0CB : register(b0);

#line 2


[numthreads(1, 1, 1)]
#line 4
void csmain(uint3 DTid : SV_DispatchThreadID)
{
	buff[0].TheInt = 1;
	buff[0].TheFloat = 1.3f;
	buff[0].TheEnum = Trinary::False;
	buff[0].TheBool = 0;
	buff[0].TheUINT = 31337;
	buff[0].TheFloat2 = float2(0.3f, 3.0f);

	buff[1].TheInt = _csmain_0CB.frameIndex;
	buff[1].TheFloat = _csmain_0CB.frameDeltaTime;
	buff[1].TheEnum = Trinary::Maybe;
	buff[1].TheBool = 2;
	buff[1].TheUINT = 255;
	buff[1].TheFloat2 = float2(99.0f, 1.0f / 99.0f);	
}

/*
Shader Resources:
	Buffer buff (as UAV)
*/
