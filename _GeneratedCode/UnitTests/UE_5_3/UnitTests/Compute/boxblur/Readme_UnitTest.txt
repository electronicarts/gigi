hooked up in post process.cpp like the readme says. I forgot to grab the exact code, so snagging it from the next unit test. it's just the wrong member name but it was this code.

	{
		simple::FTechniqueParams params;
		params.inputs.Texture_Input = SceneColor.Texture;
		simple::AddTechnique(GraphBuilder, View, params);
	}