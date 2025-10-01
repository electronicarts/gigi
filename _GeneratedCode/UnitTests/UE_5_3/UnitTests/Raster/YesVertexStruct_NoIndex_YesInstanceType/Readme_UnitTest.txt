hooked up in post process.cpp like the readme says

	if (View.ViewState)
	{
		YesVertexStruct_NoIndex_YesInstanceType::FTechniqueParams params;

		View.ViewState->TechniqueState_YesVertexStruct_NoIndex_YesInstanceType.Var_ViewProjMtx = FMatrix44f(
			FPlane4f(2.414213f, 0.000000f, 0.000000f, 0.000000f),
			FPlane4f(0.000000f, 2.414213f, 0.000000f, 0.000000f),
			FPlane4f(0.000000f, 0.000000f, -0.000100f, 0.099010f),
			FPlane4f(0.000000f, 0.000000f, 1.000000f, 10.000000f)
		);

		// Instance Buffer
		{
			static const float instanceBuffer[] =
			{
				-4.0f, 0.0f, 0.0f, 1.0f,
				0.0f, 0.0f, 0.0f, 2.0f,
				6.0f, 0.0f, 0.0f, 3.0f
			};

			params.inputs.BufferVertexFormat_Instance_Buffer.Add(FVertexElement(0, 0, VET_Float4, 3, sizeof(float) * 4));

			FRDGBufferDesc desc = FRDGBufferDesc::CreateBufferDesc(sizeof(float)*4, sizeof(instanceBuffer) / (sizeof(float)*4));
			desc.Usage |= (EBufferUsageFlags::ShaderResource);
			params.inputs.Buffer_Instance_Buffer = GraphBuilder.CreateBuffer(desc, TEXT("YesVertexStruct_NoIndex_YesInstanceType.Buffer_Instance_Buffer"));
			params.inputs.BufferFormat_Instance_Buffer = PF_R32G32B32F;

			GraphBuilder.QueueBufferUpload(params.inputs.Buffer_Instance_Buffer,
				[&]()->void*
				{
					return (void*)instanceBuffer;
				},
				[&]()->uint64
				{
					return sizeof(instanceBuffer);
				}
			);
		}

		// Vertex Buffer
		{
			struct Vertex
			{
				float pos[3];
				float normal[3];
				float uv[2];
			};

			params.inputs.BufferVertexFormat_Vertex_Buffer.Add(FVertexElement(0, 0, VET_Float3, 0, sizeof(float) * 8));
			params.inputs.BufferVertexFormat_Vertex_Buffer.Add(FVertexElement(0, sizeof(float) * 3, VET_Float3, 1, sizeof(float) * 8));
			params.inputs.BufferVertexFormat_Vertex_Buffer.Add(FVertexElement(0, sizeof(float) * 6, VET_Float2, 2, sizeof(float) * 8));

			FRDGBufferDesc desc = FRDGBufferDesc::CreateBufferDesc(sizeof(Vertex), sizeof(Bunny::bytes) / sizeof(Vertex));
			desc.Usage |= (EBufferUsageFlags::ShaderResource);
			params.inputs.Buffer_Vertex_Buffer = GraphBuilder.CreateBuffer(desc, TEXT("YesVertexStruct_NoIndex_YesInstanceType.Buffer_Vertex_Buffer"));
			params.inputs.BufferFormat_Vertex_Buffer = PF_Unknown;

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
		}

		YesVertexStruct_NoIndex_YesInstanceType::AddTechnique(GraphBuilder, View, params);
	}

To get the bunny vertices into the vertex buffer, i put "Bunny_Pos_Normal_UV.h" (in the folder that contains this folder) next to PostProcessing.cpp and added this near the top of that file:

namespace Bunny
{
	#include "Bunny_Pos_Normal_UV.h"
}

I also needed to make the color and depth target have the ShaderResource flag so they could be seen using the vis command.
Every place in the technique cpp that has ETextureCreateFlags::RenderTargetable or ETextureCreateFlags::DepthStencilTargetable
also needs ETextureCreateFlags::ShaderResource.
