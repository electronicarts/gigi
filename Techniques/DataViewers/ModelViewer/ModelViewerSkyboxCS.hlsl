// Unnamed technique, shader ModelViewerSkyboxCS
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

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
    uint2 px = DTid.xy;

	if (Depth[px].r != 0.0f || /*$(Variable:Skybox)*/ == 0)
		return;

    uint2 renderSize;
    Color.GetDimensions(renderSize.x, renderSize.y);

	// Get the world position
	float2 screenPos = (float2(px)+0.5f) / float2(renderSize) * 2.0 - 1.0;
	screenPos.y = -screenPos.y;
	float4 world = mul(float4(screenPos, 1.0f, 1.0f), /*$(Variable:InvViewProjMtx)*/);
	world.xyz /= world.w;

    float3 rayDir = normalize(world.xyz - /*$(Variable:CameraPos)*/);

    float2 uv = SampleSphericalMap(rayDir);

	float3 texel = /*$(Image:Arches_E_PineTree_3k.hdr:RGBA32_Float:float4:false)*/.SampleLevel(texSampler, uv, 0).rgb;

    Color[px] = float4(texel, 1.0f);	
}

/*
Shader Resources:
	Texture Color (as UAV)
	Texture Depth (as SRV)
*/
