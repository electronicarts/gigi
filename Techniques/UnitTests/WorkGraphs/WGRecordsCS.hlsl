// Unnamed technique, shader CS
/*$(ShaderResources)*/

/*$(_compute:main)*/(uint3 DTid : SV_DispatchThreadID)
{
    records[DTid.x].x = DTid.x;
    records[DTid.x].y = DTid.x*2;
    records[DTid.x].z = DTid.x*3;
}

/*
Shader Resources:
	Buffer records (as UAV)
*/
