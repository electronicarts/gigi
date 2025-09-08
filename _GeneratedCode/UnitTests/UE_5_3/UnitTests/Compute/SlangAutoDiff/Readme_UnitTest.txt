hooked up in post process.cpp like the readme says

	{
		static int frameIndex = 0;
		frameIndex++;
		if (View.ViewState)
			View.ViewState->TechniqueState_SlangAutoDiff.Var_FrameIndex = frameIndex;

		SlangAutoDiff::FTechniqueParams params;
		params.inputs.Texture_Output = SceneColor.Texture;
		SlangAutoDiff::AddTechnique(GraphBuilder, View, params);
	}