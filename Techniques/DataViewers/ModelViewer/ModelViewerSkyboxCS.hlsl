// Unnamed technique, shader ModelViewerSkyboxCS
/*$(ShaderResources)*/

#include "SRGB.hlsli"
#include "tonemap.hlsl"

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
    uint2 px = DTid.xy;

    if (Depth[px].r != 0.0f)
        return;

    uint2 renderSize;
    Color.GetDimensions(renderSize.x, renderSize.y);

    // Get the world position
    float2 screenPos = (float2(px) + 0.5f) / float2(renderSize) * 2.0 - 1.0;
    screenPos.y = -screenPos.y;
    float4 world = mul(float4(screenPos, 1.0f, 1.0f), /*$(Variable:InvViewProjMtx)*/);
    world.xyz /= world.w;

    float3 rayDir = normalize(world.xyz - /*$(Variable:CameraPos)*/);

    float3 color = SkyboxCubeMap.SampleLevel(LinearWrapSampler, rayDir, 0).rgb;

    // apply exposure
    color *= pow(2.0f, /*$(Variable:ExposureFStops)*/);

    // Do tonemapping
	switch(/*$(Variable:ToneMapper)*/)
	{
		// Do nothing, only apply exposure
		case ToneMappingOperation::None: break;

		// https://64.github.io/tonemapping/
		case ToneMappingOperation::Reinhard:
		{
			color = color / (1.0f + color);
			break;
		}
		case ToneMappingOperation::ACES_Luminance:
		{
			// The * 0.6f is to undo the exposure baked in, per the author's instructions
			color = ACESFilm(color * 0.6f);
			break;
		}
		case ToneMappingOperation::ACES:
		{
			color = ACESFitted(color);
			break;
		}
	}

    Color[px] = float4(LinearToSRGB(color), 1.0f);
}

/*
Shader Resources:
    Texture SkyboxCubeMap (as SRV)
	Texture Color (as UAV)
	Texture Depth (as SRV)
*/
