// DemofoxNeonDesert technique, shader Bloom Vertical and Finalize
/*$(ShaderResources)*/

#include "DemofoxNeonDesert_common.hlsl"

/*$(_compute:finalize)*/(uint3 DTid : SV_DispatchThreadID)
{
    // Does second pass of Bloom (vertical)
    // Then does exposure, tone mapping & linear to sRGB conversion.
    //fragColor = ImageFunction(iChannel0, iChannel1, fragCoord.xy, iResolution.xy);

	// get the linear color of the pixel
    float3 pixelColor = accumulation[DTid.xy].rgb;
    
    // add in bloom
    pixelColor += BloomPass2(float2(DTid.xy), scratch);
    
    // apply exposure
    pixelColor *= pow(2.0f, /*$(Variable:exposure)*/);
    
    // tone map the color to bring it from unbound HDR levels to SDR levels
	if(/*$(Variable:ACESToneMapping)*/)
    	pixelColor = ACESFilm(pixelColor);
    
    // convert to sRGB, then output
	if(/*$(Variable:sRGB)*/)
    	pixelColor = LinearToSRGB(pixelColor);
    output[DTid.xy] = float4(pixelColor, 1.0f);
}
