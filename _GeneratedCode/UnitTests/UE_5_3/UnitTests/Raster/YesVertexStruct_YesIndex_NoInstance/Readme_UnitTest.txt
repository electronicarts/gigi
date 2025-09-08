hooked up in post process.cpp like the readme says

	if (View.ViewState)
	{
		YesVertexStruct_YesIndex_NoInstance::FTechniqueParams params;

		View.ViewState->TechniqueState_YesVertexStruct_YesIndex_NoInstance.Var_ViewProjMtx = FMatrix44f(
			FPlane4f(2.414213f, 0.000000f, 0.000000f, 0.000000f),
			FPlane4f(0.000000f, 2.414213f, 0.000000f, 0.000000f),
			FPlane4f(0.000000f, 0.000000f, -0.000100f, 0.099010f),
			FPlane4f(0.000000f, 0.000000f, 1.000000f, 10.000000f)
		);

		// Vertex Buffer
		{
			struct Vertex
			{
				float pos[3];
				float uv[2];
			};

			static Vertex vertices[] =
			{
				{ { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f } },
				{ { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f } },
				{ { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f } },
				{ { 1.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } }
			};

			params.inputs.BufferVertexFormat_Vertex_Buffer.Add(FVertexElement(0, 0, VET_Float3, 0, sizeof(float) * 5));
			params.inputs.BufferVertexFormat_Vertex_Buffer.Add(FVertexElement(0, sizeof(float) * 3, VET_Float2, 1, sizeof(float) * 5));

			FRDGBufferDesc desc = FRDGBufferDesc::CreateBufferDesc(sizeof(Vertex), sizeof(vertices) / sizeof(Vertex));
			desc.Usage |= (EBufferUsageFlags::ShaderResource);
			params.inputs.Buffer_Vertex_Buffer = GraphBuilder.CreateBuffer(desc, TEXT("YesVertexStruct_YesIndex_NoInstance.Buffer_Vertex_Buffer"));
			params.inputs.BufferFormat_Vertex_Buffer = PF_Unknown;

			GraphBuilder.QueueBufferUpload(params.inputs.Buffer_Vertex_Buffer,
				[&]()->void*
				{
					return (void*)vertices;
				},
				[&]()->uint64
				{
					return sizeof(vertices);
				}
			);
		}

		// Index Buffer
		{
			static const uint32 indices[] =
			{
				0,1,2,
				1,3,2,
				2,1,0,
				2,3,1
			};

			FRDGBufferDesc desc = FRDGBufferDesc::CreateBufferDesc(sizeof(uint32), sizeof(indices) / sizeof(uint32));
			desc.Usage |= (EBufferUsageFlags::ShaderResource);
			params.inputs.Buffer_Index_Buffer = GraphBuilder.CreateBuffer(desc, TEXT("YesVertexStruct_YesIndex_NoInstance.Buffer_Index_Buffer"));
			params.inputs.BufferFormat_Index_Buffer = PF_R32_UINT;

			GraphBuilder.QueueBufferUpload(params.inputs.Buffer_Index_Buffer,
				[&]()->void*
				{
					return (void*)indices;
				},
				[&]()->uint64
				{
					return sizeof(indices);
				}
			);
		}

		YesVertexStruct_YesIndex_NoInstance::AddTechnique(GraphBuilder, View, params);
	}


I also needed to make the color and depth target have the ShaderResource flag so they could be seen using the vis command.
Every place in the technique cpp that has ETextureCreateFlags::RenderTargetable or ETextureCreateFlags::DepthStencilTargetable
also needs ETextureCreateFlags::ShaderResource.
