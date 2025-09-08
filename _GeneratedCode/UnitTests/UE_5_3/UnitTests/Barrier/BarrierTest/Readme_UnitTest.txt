hooked up in post process.cpp like the readme says

	{
		BarrierTest::FTechniqueParams params;
		BarrierTest::AddTechnique(GraphBuilder, View, params);
		SceneColor.Texture = params.outputs.Texture_Output;
	}

I did need to make sure the texture create flags included shader resource though. I changed line 79 in BarrierTestTechnique.cpp to this:

ETextureCreateFlags createFlags = ETextureCreateFlags::UAV | ETextureCreateFlags::ShaderResource;

Without that, the code that used SceneColor.texture would crash, and so would the vis command, if i didn't set the texture output as the scene color texture.