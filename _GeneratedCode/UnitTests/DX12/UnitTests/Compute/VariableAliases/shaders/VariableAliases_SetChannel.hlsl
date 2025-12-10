// Unnamed technique, shader SetChannel


struct Struct__SetChannel_0CB
{
    int _alias_Channel;
    float _alias_Value;
    float2 _padding0;
};

RWTexture2D<float4> Output : register(u0);
ConstantBuffer<Struct__SetChannel_0CB> _SetChannel_0CB : register(b0);

int _GetVariableAliasValue_Channel()
{
    #ifdef __GIGI_AlIAS_VARIABLE_CONST_Channel
        return __GIGI_AlIAS_VARIABLE_CONST_Channel;
    #else
        return _SetChannel_0CB._alias_Channel;
    #endif
}

float _GetVariableAliasValue_Value()
{
    #ifdef __GIGI_AlIAS_VARIABLE_CONST_Value
        return __GIGI_AlIAS_VARIABLE_CONST_Value;
    #else
        return _SetChannel_0CB._alias_Value;
    #endif
}

#line 2


[numthreads(8, 8, 1)]
#line 4
void main(uint3 DTid : SV_DispatchThreadID)
{
	// Get the pixel color
	uint2 px = DTid.xy;
	float3 pixelValue = Output[px].rgb;

	// Set the specified channel to the specified value
	pixelValue[_GetVariableAliasValue_Channel()] = _GetVariableAliasValue_Value();

	// Write it back
	Output[px] = float4(pixelValue, 1.0);
}

/*
Shader Resources:
	Texture Output (as UAV)
*/
