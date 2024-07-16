/*$(ShaderResources)*/

float3 accurateLinearToSRGB(float3 linearCol)
{
	float3 sRGBLo = linearCol * 12.92;
	float3 sRGBHi = (pow(abs(linearCol), float3(1.0 / 2.4, 1.0 / 2.4, 1.0 / 2.4)) * 1.055) - 0.055;
	//float3 sRGB = select(linearCol <= 0.0031308, sRGBLo, sRGBHi);
	float3 sRGB;
	sRGB.r = linearCol.r <= 0.0031308 ? sRGBLo.r : sRGBHi.r;
	sRGB.g = linearCol.g <= 0.0031308 ? sRGBLo.g : sRGBHi.g;
	sRGB.b = linearCol.b <= 0.0031308 ? sRGBLo.b : sRGBHi.b;
	return sRGB;
}

float3 accurateSRGBToLinear(in float3 sRGBCol)
{
	float3 linearRGBLo = sRGBCol / 12.92;
	float3 linearRGBHi = pow((sRGBCol + 0.055) / 1.055, float3(2.4, 2.4, 2.4));
	//float3 linearRGB = select(sRGBCol <= 0.04045, linearRGBLo, linearRGBHi);
	float3 linearRGB;
	linearRGB.r = sRGBCol.r <= 0.04045 ? linearRGBLo.r : linearRGBHi.r;
	linearRGB.g = sRGBCol.g <= 0.04045 ? linearRGBLo.g : linearRGBHi.g;
	linearRGB.b = sRGBCol.b <= 0.04045 ? linearRGBLo.b : linearRGBHi.b;
	return linearRGB;
}

void WriteTexture(uint2 px, in Texture2D<float4> t)
{
    uint width, height;
    t.GetDimensions(width, height);

    uint2 srcpx = px;
    if (/*$(Variable:Tile)*/)
    {
        srcpx = px % uint2(width, height);
    }
    else
    {
        if (px.x >= width || px.y >= height)
            return;
    }

    float3 result = accurateLinearToSRGB(lerp(accurateSRGBToLinear(RenderTarget[px].rgb), t[srcpx].rgb, /*$(Variable:Opacity)*/));
    RenderTarget[px] = float4(result, 1.0f);
}

/*$(_compute:Main)*/(uint3 DTid : SV_DispatchThreadID)
{
    switch(/*$(Variable:Texture)*/)
    {
        case Textures::Logo: WriteTexture(DTid.xy, /*$(Image:logo.png:RGBA8_Unorm_sRGB:float4:true)*/);break;
        case Textures::Cabin: WriteTexture(DTid.xy, /*$(Image:cabinsmall.png:RGBA8_Unorm_sRGB:float4:true)*/);break;
    }
}
