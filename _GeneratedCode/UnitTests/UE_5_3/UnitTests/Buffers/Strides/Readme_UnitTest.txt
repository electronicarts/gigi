I installed it the usual way but then did this for readback to verify results:

	// Do readback
	{
		static FRHIGPUBufferReadback* Readback = nullptr;
		if (!Readback)
		{
			Readback = new FRHIGPUBufferReadback(FName(TEXT("Strides Readback")));
			AddEnqueueCopyPass(GraphBuilder, Readback, Buffer_TheBuffer, 32 * 4);
		}

		static FRHIGPUBufferReadback* Readback2 = nullptr;
		if (!Readback2)
		{
			Readback2 = new FRHIGPUBufferReadback(FName(TEXT("Strides Readback2")));
			AddEnqueueCopyPass(GraphBuilder, Readback2, Buffer_TheBuffer2, 32 * 4);
		}

		if (Readback && Readback->IsReady())
		{
			const void* mappedData = Readback->Lock(32 * 4);

			if (mappedData)
			{
				const uint8* data = (const uint8*)mappedData;

				Readback->Unlock();
				AddEnqueueCopyPass(GraphBuilder, Readback, Buffer_TheBuffer, 32 * 4);
			}
		}

		if (Readback2 && Readback2->IsReady())
		{
			const void* mappedData = Readback2->Lock(32 * 4);

			if (mappedData)
			{
				const uint8* data = (const uint8*)mappedData;

				Readback2->Unlock();
				AddEnqueueCopyPass(GraphBuilder, Readback2, Buffer_TheBuffer2, 32 * 4);
			}
		}
	}