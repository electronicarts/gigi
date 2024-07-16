// IBL technique, shader NoShadows
/*$(ShaderResources)*/

#include "GBuffer.hlsl"
#include "PBR.hlsl"

/*$(_compute:csmain)*/(uint3 DTid : SV_DispatchThreadID)
{
	// If light is off, nothing to do
	if (!/*$(Variable:Enabled)*/)
		return;

	uint2 dims;
	Output.GetDimensions(dims.x, dims.y);

	// If this is empty sky, show the skybox and exit out
	uint2 px = DTid.xy;
	float depth = Depth[px];
	if (depth == /*$(Variable:DepthClearValue)*/)
	{
		uint2 renderSize;
		Output.GetDimensions(renderSize.x, renderSize.y);

		// Get the world position
		float2 screenPos = (float2(px)+0.5f) / float2(renderSize) * 2.0 - 1.0;
		screenPos.y = -screenPos.y;
		float4 world = mul(float4(screenPos, 1.0f, 1.0f), /*$(Variable:InvViewProjMtx)*/);
		world.xyz /= world.w;
		float3 rayDir = normalize(world.xyz - /*$(Variable:CameraPos)*/);

		switch(/*$(Variable:Skybox)*/)
		{
			case Skyboxes::Vasa: Output[px] = float4(/*$(ImageCube:Skyboxes/Vasa/Vasa%s.png:RGBA8_UNorm:float4:true)*/.SampleLevel(LinearWrap, rayDir, 0).rgb, 1.0f); return;
			case Skyboxes::AshCanyon: Output[px] = float4(/*$(ImageCube:Skyboxes/AshCanyon/AshCanyon%s.png:RGBA8_UNorm:float4:true)*/.SampleLevel(LinearWrap, rayDir, 0).rgb, 1.0f); return;
			case Skyboxes::Marriot: Output[px] = float4(/*$(ImageCube:Skyboxes/Marriot/Marriot%s.png:RGBA8_UNorm:float4:true)*/.SampleLevel(LinearWrap, rayDir, 0).rgb, 1.0f); return;
		}
		return;
	}

	// Becode gbuffer
	GBuffer gbuffer = DecodeGBuffer(Albedo_Roughness[px], Normal_Metallic_AO[px]);

	// Get world position of pixel
	float2 screenPos = (float2(px)+0.5f) / float2(dims) * 2.0 - 1.0;
	screenPos.y = -screenPos.y;
	float4 world = mul(float4(screenPos, depth, 1), /*$(Variable:InvViewProjMtx)*/);
	world /= world.w;

	// Get reverse view direction (from world to camera)
	float3 V = normalize(/*$(Variable:CameraPos)*/ - world.xyz);

	// Get the reflection direction
	float3 R = reflect(-V, gbuffer.normal);

	float3 litColor = Output[px].rgb;

	switch(/*$(Variable:Skybox)*/)
	{
		case Skyboxes::Vasa:
		{
			TextureCube<float4> DiffuseIBL   = /*$(ImageCube:Skyboxes/Vasa/VasaDiffuse%s.png:RGBA8_UNorm:float4:true)*/;
			TextureCube<float4> SpecularIBL0 = /*$(ImageCube:Skyboxes/Vasa/Vasa0Specular%s.png:RGBA8_UNorm:float4:true)*/;
			TextureCube<float4> SpecularIBL1 = /*$(ImageCube:Skyboxes/Vasa/Vasa1Specular%s.png:RGBA8_UNorm:float4:true)*/;
			TextureCube<float4> SpecularIBL2 = /*$(ImageCube:Skyboxes/Vasa/Vasa2Specular%s.png:RGBA8_UNorm:float4:true)*/;
			TextureCube<float4> SpecularIBL3 = /*$(ImageCube:Skyboxes/Vasa/Vasa3Specular%s.png:RGBA8_UNorm:float4:true)*/;
			TextureCube<float4> SpecularIBL4 = /*$(ImageCube:Skyboxes/Vasa/Vasa4Specular%s.png:RGBA8_UNorm:float4:true)*/;
			litColor += IBL(gbuffer.normal, V, R, gbuffer.albedo, gbuffer.metallic, gbuffer.roughness, c_F0, /*$(Image2D:Textures/splitsum.png:RGBA8_UNorm:float4:false)*/, DiffuseIBL, SpecularIBL0, SpecularIBL1, SpecularIBL2, SpecularIBL3, SpecularIBL4, LinearWrap) * gbuffer.ao;
			break;
		}
		case Skyboxes::AshCanyon:
		{
			TextureCube<float4> DiffuseIBL   = /*$(ImageCube:Skyboxes/AshCanyon/AshCanyonDiffuse%s.png:RGBA8_UNorm:float4:true)*/;
			TextureCube<float4> SpecularIBL0 = /*$(ImageCube:Skyboxes/AshCanyon/AshCanyon0Specular%s.png:RGBA8_UNorm:float4:true)*/;
			TextureCube<float4> SpecularIBL1 = /*$(ImageCube:Skyboxes/AshCanyon/AshCanyon1Specular%s.png:RGBA8_UNorm:float4:true)*/;
			TextureCube<float4> SpecularIBL2 = /*$(ImageCube:Skyboxes/AshCanyon/AshCanyon2Specular%s.png:RGBA8_UNorm:float4:true)*/;
			TextureCube<float4> SpecularIBL3 = /*$(ImageCube:Skyboxes/AshCanyon/AshCanyon3Specular%s.png:RGBA8_UNorm:float4:true)*/;
			TextureCube<float4> SpecularIBL4 = /*$(ImageCube:Skyboxes/AshCanyon/AshCanyon4Specular%s.png:RGBA8_UNorm:float4:true)*/;
			litColor += IBL(gbuffer.normal, V, R, gbuffer.albedo, gbuffer.metallic, gbuffer.roughness, c_F0, /*$(Image2D:Textures/splitsum.png:RGBA8_UNorm:float4:false)*/, DiffuseIBL, SpecularIBL0, SpecularIBL1, SpecularIBL2, SpecularIBL3, SpecularIBL4, LinearWrap) * gbuffer.ao;
			break;
		}
		case Skyboxes::Marriot:
		{
			TextureCube<float4> DiffuseIBL   = /*$(ImageCube:Skyboxes/Marriot/MarriotDiffuse%s.png:RGBA8_UNorm:float4:true)*/;
			TextureCube<float4> SpecularIBL0 = /*$(ImageCube:Skyboxes/Marriot/Marriot0Specular%s.png:RGBA8_UNorm:float4:true)*/;
			TextureCube<float4> SpecularIBL1 = /*$(ImageCube:Skyboxes/Marriot/Marriot1Specular%s.png:RGBA8_UNorm:float4:true)*/;
			TextureCube<float4> SpecularIBL2 = /*$(ImageCube:Skyboxes/Marriot/Marriot2Specular%s.png:RGBA8_UNorm:float4:true)*/;
			TextureCube<float4> SpecularIBL3 = /*$(ImageCube:Skyboxes/Marriot/Marriot3Specular%s.png:RGBA8_UNorm:float4:true)*/;
			TextureCube<float4> SpecularIBL4 = /*$(ImageCube:Skyboxes/Marriot/Marriot4Specular%s.png:RGBA8_UNorm:float4:true)*/;
			litColor += IBL(gbuffer.normal, V, R, gbuffer.albedo, gbuffer.metallic, gbuffer.roughness, c_F0, /*$(Image2D:Textures/splitsum.png:RGBA8_UNorm:float4:false)*/, DiffuseIBL, SpecularIBL0, SpecularIBL1, SpecularIBL2, SpecularIBL3, SpecularIBL4, LinearWrap) * gbuffer.ao;
			break;
		}
	}
	Output[px] = float4(litColor, 1.0f);	
}

/*
Shader Resources:
	Texture Depth (as SRV)
	Texture Albedo_Roughness (as SRV)
	Texture Normal_Metallic_AO (as SRV)
	Texture Output (as UAV)
*/
