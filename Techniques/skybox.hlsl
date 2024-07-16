/*$(ShaderResources)*/

// from https://learnopengl.com/PBR/IBL/Diffuse-irradiance
float2 SampleSphericalMap(float3 v)
{
    const float2 invAtan = float2(0.1591f, 0.3183f);
    float2 uv = float2(atan2(v.z, v.x), asin(-v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}

float3 LinearToSRGB(float3 linearCol)
{
	float3 sRGBLo = linearCol * 12.92;
	float3 sRGBHi = (pow(abs(linearCol), float3(1.0 / 2.4, 1.0 / 2.4, 1.0 / 2.4)) * 1.055) - 0.055;
	float3 sRGB;
	sRGB.r = linearCol.r <= 0.0031308 ? sRGBLo.r : sRGBHi.r;
	sRGB.g = linearCol.g <= 0.0031308 ? sRGBLo.g : sRGBHi.g;
	sRGB.b = linearCol.b <= 0.0031308 ? sRGBLo.b : sRGBHi.b;
	return sRGB;
}

/*$(_compute:skyboxCS)*/(uint3 DTid : SV_DispatchThreadID)
{
    uint2 px = DTid.xy;

    uint2 renderSize;
    Color.GetDimensions(renderSize.x, renderSize.y);

	// Get the world position
	float2 screenPos = (float2(px)+0.5f) / float2(renderSize) * 2.0 - 1.0;
	screenPos.y = -screenPos.y;
	float4 world = mul(float4(screenPos, 1.0f, 1.0f), /*$(Variable:clipToView)*/);
	world.xyz /= world.w;

    float3 rayDir = normalize(world.xyz - /*$(Variable:cameraPos)*/);

    float2 uv = SampleSphericalMap(rayDir);

    float3 texel = float3(0.0f, 0.0f, 0.0f);
    switch(/*$(Variable:skybox)*/)
    {
        case Skybox::Arches: texel = /*$(Image:skyboxes/Arches_E_PineTree_3k.hdr:RGBA32_Float:float4:false)*/.SampleLevel(texSampler, uv, 0).rgb; break;
        case Skybox::SataraNightHdr: texel = /*$(Image:skyboxes/satara_night_4k.hdr:RGBA32_Float:float4:false)*/.SampleLevel(texSampler, uv, 0).rgb; break;
        case Skybox::SataraNightExr: texel = /*$(Image:skyboxes/satara_night_4k.exr:RGBA32_Float:float4:false)*/.SampleLevel(texSampler, uv, 0).rgb; break;
    }

    float exposure = pow(2.0f, /*$(Variable:exposureFStops)*/);
    texel *= exposure;

    texel = LinearToSRGB(texel);

    Color[px] = float4(texel, 1.0f);
}
