hooked up in post process.cpp like the readme says

	{
		if (View.ViewState)
		{
			static int frameIndex = -1;
			frameIndex++;
			View.ViewState->TechniqueState_ReadbackSequence.Var_frameIndex = frameIndex;
		}

		ReadbackSequence::FTechniqueParams params;
		params.inputs.Texture_Output = SceneColor.Texture;
		ReadbackSequence::AddTechnique(GraphBuilder, View, params);
	}