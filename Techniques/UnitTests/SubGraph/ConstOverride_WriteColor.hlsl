// Unnamed technique, shader WriteColorCS
/*$(ShaderResources)*/

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= /*$(Variable:MinX)*/ && DTid.x <= /*$(Variable:MaxX)*/)
        Color[DTid.xy] = float4(0.2f, 0.8f, 0.2f, 1.0f);
}

/*
Shader Resources:
	Texture Color (as UAV)
*/
