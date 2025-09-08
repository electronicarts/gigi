hooked up in post process.cpp like the readme says

	{
		buffertest::FTechniqueParams params;

		struct DataStruct
		{
			float TheFloat4[4];
			int TheInt4[4];
			int TheBool;
		};

		static const DataStruct structData =
		{
			{4.0f, 2.0f, 3.0f, 1.0f},
			{104, 103, 102, 101},
			true
		};

		static const float typedData[] = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

		// Buffer_InputTypedBuffer
		{
			FRDGBufferDesc desc = FRDGBufferDesc::CreateBufferDesc(sizeof(float), 8);
			desc.Usage |= (EBufferUsageFlags::ShaderResource);
			params.inputs.Buffer_InputTypedBuffer = GraphBuilder.CreateBuffer(desc, TEXT("Buffer_InputTypedBuffer"));
			params.inputs.BufferFormat_InputTypedBuffer = PF_R32_FLOAT;

			GraphBuilder.QueueBufferUpload(params.inputs.Buffer_InputTypedBuffer,
				[&]()->void*
				{
					return (void*)typedData;
				},
				[&]()->uint64
				{
					return sizeof(typedData);
				}
			);
		}

		// Buffer_InputStructuredBuffer
		{
			FRDGBufferDesc desc = FRDGBufferDesc::CreateBufferDesc(sizeof(float), 9);
			desc.Usage |= (EBufferUsageFlags::ShaderResource | EBufferUsageFlags::StructuredBuffer);
			params.inputs.Buffer_InputStructuredBuffer = GraphBuilder.CreateBuffer(desc, TEXT("Buffer_InputStructuredBuffer"));
			params.inputs.BufferFormat_InputStructuredBuffer = PF_Unknown;

			GraphBuilder.QueueBufferUpload(params.inputs.Buffer_InputStructuredBuffer,
				[&]()->void*
				{
					return (void*)&structData;
				},
				[&]()->uint64
				{
					return sizeof(structData);
				}
			);
		}

		// Buffer_InputTypedBufferRaw
		{
			FRDGBufferDesc desc = FRDGBufferDesc::CreateBufferDesc(sizeof(float), 8);
			desc.Usage |= (EBufferUsageFlags::ShaderResource | EBufferUsageFlags::ByteAddressBuffer);
			params.inputs.Buffer_InputTypedBufferRaw = GraphBuilder.CreateBuffer(desc, TEXT("Buffer_InputTypedBufferRaw"));
			params.inputs.BufferFormat_InputTypedBufferRaw = PF_R32_FLOAT;

			GraphBuilder.QueueBufferUpload(params.inputs.Buffer_InputTypedBufferRaw,
				[&]()->void*
				{
					return (void*)typedData;
				},
				[&]()->uint64
				{
					return sizeof(typedData);
				}
			);
		}

		buffertest::AddTechnique(GraphBuilder, View, params);
	}