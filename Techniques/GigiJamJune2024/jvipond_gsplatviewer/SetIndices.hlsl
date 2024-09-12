// Unnamed technique, shader SetIndices
/*$(ShaderResources)*/

/*$(_compute:SetIndices)*/(uint3 DTid : SV_DispatchThreadID)
{
    uint idx = DTid.x;
    if (idx >= /*$(Variable:_SplatCount)*/)
        return;

    _SplatSortKeys[idx] = idx;
}

/*
Shader Resources:
	Buffer _SplatSortKeys (as UAV)
*/
