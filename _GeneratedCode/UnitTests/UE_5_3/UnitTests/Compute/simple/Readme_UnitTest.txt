hooked up in post process.cpp like the readme says

	{
		simple::FTechniqueParams params;
		params.inputs.Texture_Input = SceneColor.Texture;
		simple::AddTechnique(GraphBuilder, View, params);
	}