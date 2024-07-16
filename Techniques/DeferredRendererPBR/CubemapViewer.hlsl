/*$(ShaderResources)*/

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
	float4 world = mul(float4(screenPos, 1.0f, 1.0f), /*$(Variable:InvViewProjMtx)*/);
	world.xyz /= world.w;

    float3 rayDir = normalize(world.xyz - /*$(Variable:CameraPos)*/);

    float3 texel = float3(0.0f, 0.0f, 0.0f);
    switch(/*$(Variable:Skybox)*/)
    {
        case SkyboxType::Vasa:
        {
            switch(/*$(Variable:View)*/)
            {
                case ViewType::Skybox:    texel = /*$(ImageCube:Skyboxes\Vasa\Vasa%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Specular0: texel = /*$(ImageCube:Skyboxes\Vasa\Vasa0Specular%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Specular1: texel = /*$(ImageCube:Skyboxes\Vasa\Vasa1Specular%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Specular2: texel = /*$(ImageCube:Skyboxes\Vasa\Vasa2Specular%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Specular3: texel = /*$(ImageCube:Skyboxes\Vasa\Vasa3Specular%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Specular4: texel = /*$(ImageCube:Skyboxes\Vasa\Vasa4Specular%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Diffuse:   texel = /*$(ImageCube:Skyboxes\Vasa\VasaDiffuse%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
            }
            break;
        }
        case SkyboxType::Marriot:
        {
            switch(/*$(Variable:View)*/)
            {
                case ViewType::Skybox:    texel = /*$(ImageCube:Skyboxes\Marriot\Marriot%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Specular0: texel = /*$(ImageCube:Skyboxes\Marriot\Marriot0Specular%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Specular1: texel = /*$(ImageCube:Skyboxes\Marriot\Marriot1Specular%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Specular2: texel = /*$(ImageCube:Skyboxes\Marriot\Marriot2Specular%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Specular3: texel = /*$(ImageCube:Skyboxes\Marriot\Marriot3Specular%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Specular4: texel = /*$(ImageCube:Skyboxes\Marriot\Marriot4Specular%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Diffuse:   texel = /*$(ImageCube:Skyboxes\Marriot\MarriotDiffuse%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
            }
            break;
        }
        case SkyboxType::AshCanyon:
        {
            switch(/*$(Variable:View)*/)
            {
                case ViewType::Skybox:    texel = /*$(ImageCube:Skyboxes\ashcanyon\ashcanyon%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Specular0: texel = /*$(ImageCube:Skyboxes\ashcanyon\ashcanyon0Specular%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Specular1: texel = /*$(ImageCube:Skyboxes\ashcanyon\ashcanyon1Specular%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Specular2: texel = /*$(ImageCube:Skyboxes\ashcanyon\ashcanyon2Specular%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Specular3: texel = /*$(ImageCube:Skyboxes\ashcanyon\ashcanyon3Specular%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Specular4: texel = /*$(ImageCube:Skyboxes\ashcanyon\ashcanyon4Specular%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
                case ViewType::Diffuse:   texel = /*$(ImageCube:Skyboxes\ashcanyon\ashcanyonDiffuse%s.png:RGBA8_Unorm_sRGB:float4:true)*/.SampleLevel(texSampler, rayDir, 0).rgb; break;
            }
            break;
        }
    }

    texel *= pow(2.0f, /*$(Variable:exposureFStops)*/);;
    Color[px] = float4(LinearToSRGB(texel), 1.0f);
}
