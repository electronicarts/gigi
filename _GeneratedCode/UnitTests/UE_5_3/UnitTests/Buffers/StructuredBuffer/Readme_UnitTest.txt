hooked up in post process.cpp like the readme says

	{
		StructuredBuffer::FTechniqueParams params;
		static StructuredBuffer::Struct_TheStructure data[2];

		// Buffer_InputStructuredBuffer
		{
			FRDGBufferDesc desc = FRDGBufferDesc::CreateBufferDesc(sizeof(float), 9);
			desc.Usage |= (EBufferUsageFlags::ShaderResource | EBufferUsageFlags::StructuredBuffer);
			params.inputs.Buffer_buff = GraphBuilder.CreateBuffer(desc, TEXT("Buffer_buff"));
			params.inputs.BufferFormat_buff = PF_Unknown;

			GraphBuilder.QueueBufferUpload(params.inputs.Buffer_buff,
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

		StructuredBuffer::AddTechnique(GraphBuilder, View, params);
	}