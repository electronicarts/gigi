// Unnamed technique, shader RenderSplatVertex
/*$(ShaderResources)*/

struct v2f // AKA PSInput
{
    half4 col : COLOR0;
    float2 pos : TEXCOORD0;
    float4 vertex : SV_POSITION;
};

v2f vert(uint vtxID : SV_VertexID, uint instID : SV_InstanceID)
{
    v2f o = (v2f) 0;
    instID = _OrderBuffer[instID];
    Struct_GaussianSplatViewData view = _SplatViewData[instID];
    float4 centerClipPos = view.pos;
    bool behindCam = centerClipPos.w <= 0;
    if (behindCam)
    {
        o.vertex = asfloat(0x7fc00000); // NaN discards the primitive
    }
    else
    {
        o.col.r = f16tof32(view.color1.x >> 16);
        o.col.g = f16tof32(view.color1.x);
        o.col.b = f16tof32(view.color2.x >> 16);
        o.col.a = f16tof32(view.color2.x);

        uint idx = vtxID;
        float2 quadPos = float2(idx & 1, (idx >> 1) & 1) * 2.0 - 1.0;
        quadPos *= 2;

        o.pos = quadPos;

        float2 deltaScreenPos = (quadPos.x * view.axis1 + quadPos.y * view.axis2) * 2 / /*$(Variable:RenderSize)*/.xy;
        o.vertex = centerClipPos;
        o.vertex.xy += deltaScreenPos * centerClipPos.w;

		// is this splat selected?
		//if (_SplatBitsValid)
		//{
		//	uint wordIdx = instID / 32;
		//	uint bitIdx = instID & 31;
		//	uint selVal = _SplatSelectedBits.Load(wordIdx * 4);
		//	if (selVal & (1 << bitIdx))
		//	{
		//		o.col.a = -1;				
		//	}
		//}
    }
    return o;
}

/*
Shader Resources:
	Buffer _OrderBuffer (as SRV)
	Buffer _SplatViewData (as SRV)
*/
