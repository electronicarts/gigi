hooked up in post process.cpp like the readme says

	if (View.ViewState)
	{
		View.ViewState->TechniqueState_YesVertexType_NoIndex_NoInstance.Var_ViewProjMtx = FMatrix44f(
			FPlane4f(2.414213f, 0.000000f, 0.000000f, 0.000000f),
			FPlane4f(0.000000f, 2.414213f, 0.000000f, 0.000000f),
			FPlane4f(0.000000f, 0.000000f, -0.000100f, 0.099010f),
			FPlane4f(0.000000f, 0.000000f, 1.000000f, 10.000000f)
		);

		YesVertexType_NoIndex_NoInstance::FTechniqueParams params;

		params.inputs.BufferVertexFormat_Vertex_Buffer.Add(FVertexElement(0, 0, VET_Float3, 0, sizeof(float) * 3));

		FRDGBufferDesc desc = FRDGBufferDesc::CreateBufferDesc(sizeof(float)*3, sizeof(Bunny::bytes) / (sizeof(float)*3));
		desc.Usage |= (EBufferUsageFlags::ShaderResource);
		params.inputs.Buffer_Vertex_Buffer = GraphBuilder.CreateBuffer(desc, TEXT("YesVertexType_NoIndex_NoInstance.Buffer_Vertex_Buffer"));
		params.inputs.BufferFormat_Vertex_Buffer = PF_R32G32B32F;

		GraphBuilder.QueueBufferUpload(params.inputs.Buffer_Vertex_Buffer,
			[&]()->void*
			{
				return (void*)Bunny::bytes;
			},
			[&]()->uint64
			{
				return sizeof(Bunny::bytes);
			}
		);

		YesVertexType_NoIndex_NoInstance::AddTechnique(GraphBuilder, View, params);
	}

To get the bunny vertices into the vertex buffer, i put "Bunny_Pos3F.h" (in the folder that contains this folder) next to PostProcessing.cpp and added this near the top of that file:

namespace Bunny
{
	#include "Bunny_Pos3F.h"
}

I also needed to make the color and depth target have the ShaderResource flag so they could be seen using the vis command.

line 66:
ETextureCreateFlags createFlags = ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource;

line 92:
ETextureCreateFlags createFlags = ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource;

line 162:
psoDesc.RenderTargetFlags[0] = ETextureCreateFlags::RenderTargetable | ETextureCreateFlags::ShaderResource;

line 169:
psoDesc.DepthStencilTargetFlag = ETextureCreateFlags::DepthStencilTargetable | ETextureCreateFlags::ShaderResource;