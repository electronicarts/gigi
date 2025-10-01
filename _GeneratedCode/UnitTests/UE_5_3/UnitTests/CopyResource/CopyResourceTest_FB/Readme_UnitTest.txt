hooked up in post process.cpp like the readme says

	{
		CopyResourceTest_FB::FTechniqueParams params;

		// Buffer_Source_Buffer
		{
			static const float data[] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

			FRDGBufferDesc desc = FRDGBufferDesc::CreateBufferDesc(sizeof(float), 8);
			desc.Usage |= (EBufferUsageFlags::ShaderResource);
			params.inputs.Buffer_Source_Buffer = GraphBuilder.CreateBuffer(desc, TEXT("Buffer_Source_Buffer"));
			params.inputs.BufferFormat_Source_Buffer = PF_R32_FLOAT;

			GraphBuilder.QueueBufferUpload(params.inputs.Buffer_InputTypedBuffer,
				[&]()->void*
				{
					return (void*)data;
				},
				[&]()->uint64
				{
					return sizeof(data);
				}
			);
		}

		params.inputs.Texture_Source_Texture = SceneColor.Texture;
		CopyResourceTest_FB::AddTechnique(GraphBuilder, View, params);
	}
