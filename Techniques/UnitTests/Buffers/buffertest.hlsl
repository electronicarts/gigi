// Buffer test Shader/*$(ShaderResources)*/
/*$(_compute:Main)*/(uint3 DTid : SV_DispatchThreadID)
{
	// Do FIR on the typed buffer
	// Info: https://blog.demofox.org/2020/01/14/fir-audio-data-filters/
	// Coefficient Calculator: http://demofox.org/DSPFIR/FIR.html
	{
		float n_minus_2 = (DTid.x >= 2) ? InputTyped[DTid.x-2] : 0.0f;
		float n_minus_1 = (DTid.x >= 1) ? InputTyped[DTid.x-1] : 0.0f;
		float n = InputTyped[DTid.x];
		OutputTyped[DTid.x] = /*$(Variable:gain)*/ * (n + /*$(Variable:alpha1)*/ * n_minus_1 + /*$(Variable:alpha2)*/ * n_minus_2);
	}

	{
		float n_minus_2 = (DTid.x >= 2) ? InputTypedStruct[DTid.x - 2] : 0.0f;
		float n_minus_1 = (DTid.x >= 1) ? InputTypedStruct[DTid.x - 1] : 0.0f;
		float n = InputTypedStruct[DTid.x];
		OutputTypedStruct[DTid.x] = /*$(Variable:gain)*/ * (n + /*$(Variable:alpha1)*/ * n_minus_1 + /*$(Variable:alpha2)*/ * n_minus_2);
	}

	if (DTid.x == 0)
	{
		// modify the structured buffer
		{
			Struct_TestStruct s = InputStructured[0];

			s.TheFloat4.x += 0.1f;
			s.TheFloat4.y += 0.2f;
			s.TheFloat4.z += 0.3f;
			s.TheFloat4.w += 0.4f;

			s.TheInt4.x += 1;
			s.TheInt4.y += 2;
			s.TheInt4.z += 3;
			s.TheInt4.w += 4;

			s.TheBool = !s.TheBool;

			OutputStructured[0] = s;
		}

		// modify the raw byte buffers
		OutputTypedRaw.Store(4, asuint(asfloat(InputTypedRaw.Load(4))+10.0f));
	}
}
