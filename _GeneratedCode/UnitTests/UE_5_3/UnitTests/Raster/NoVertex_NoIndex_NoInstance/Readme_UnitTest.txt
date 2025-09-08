hooked up in post process.cpp like the readme says

        if (View.ViewState)
 	{
		View.ViewState->TechniqueState_NoVertex_NoIndex_NoInstance.Var_ViewProjMtx = FMatrix44f(
			FPlane4f(2.414213f, 0.000000f, 0.000000f, 0.000000f),
			FPlane4f(0.000000f, 2.414213f, 0.000000f, 0.000000f),
			FPlane4f(0.000000f, 0.000000f, -0.000100f, 0.099010f),
			FPlane4f(0.000000f, 0.000000f, 1.000000f, 10.000000f)
		);

 		NoVertex_NoIndex_NoInstance::FTechniqueParams params;
 		NoVertex_NoIndex_NoInstance::AddTechnique(GraphBuilder, View, params);
 	}

I also needed to make the color and depth target have the ShaderResource flag so they could be seen using the vis command.

line 65:
ETextureCreateFlags createFlags = ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource;

line 91:
ETextureCreateFlags createFlags = ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource;

line 156:
psoDesc.RenderTargetFlags[0] = ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource;

line 163:
psoDesc.DepthStencilTargetFlag = ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource;

