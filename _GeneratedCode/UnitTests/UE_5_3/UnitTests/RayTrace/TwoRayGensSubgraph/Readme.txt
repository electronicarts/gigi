This package was generated for compatibility with Unreal Engine 5.3.2

=====================================================================================================
Add the Files To Unreal
=====================================================================================================

You can follow the steps below manually, or run install.bat, with the path to the unreal source code
root - the folder where GenerateProjectFiles.bat lives.

Make sure and run GenerateProjectFiles.bat to regenerate the unreal solution to include these new files.

Copy Public/TwoRayGensSubgraphTechnique.h to Engine/Source/Runtime/Renderer/Public/

Copy Public/TwoRayGensSubgraphTechnique.cpp to Engine/Source/Runtime/Renderer/Private/

Copy the contents of Shaders to Engine/Shaders/Private/TwoRayGensSubgraph/

=====================================================================================================
Set Up Non Transient Storage
=====================================================================================================

This technique requires non transient storage.

To set that up, put this struct into Engine\Source\Runtime\Renderer\Private\ScenePrivate.h:

struct FTechniqueState_TwoRayGensSubgraph
{
    // Host Variables - feel free to read and write these
    FMatrix44f Var_clipToWorld = FMatrix44f(FPlane4f(0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f), FPlane4f(0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f), FPlane4f(0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f), FPlane4f(0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f));
    FVector3f Var_cameraPos = FVector3f(0.f, 0.f, 0.f);
    float Var_depthNearPlane = float(0.f);  // The depth value for the near plane.
    FVector3f Var_hitColor = FVector3f(0.f, 1.f, 0.f);
    FVector3f Var_missColor = FVector3f(1.f, 0.f, 0.f);
};

and put this member variable into FSceneViewState:

FTechniqueState_TwoRayGensSubgraph TechniqueState_TwoRayGensSubgraph;

=====================================================================================================
Connect Technique
=====================================================================================================

The location of where to run the technique, what inputs to give, and what to do with the outputs is
very much specific to the details of the technique and the usage case. However, here is how you
would connect a new post processing effect, as an example.

in Engine/Source/Runtime/Renderer/Private/PostProcess/PostProcessing.cpp:

1) include your header near the top of the file

#include "TwoRayGensSubgraphTechnique.h"

2) Call into your technique from inside AddPostProcessingPasses()

Around line 318, there will be this line getting the SceneColor texture.

FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, PrimaryViewRect);

Assuming you want your new post processing effect to run first, you can put this chunk of code after that line:

	{
		TwoRayGensSubgraph::FTechniqueParams params;

		// Set inputs, such as this line:
		//params.inputs.Texture_InputTexture = SceneColor.Texture;

		// Set variables, such as this line:
		// View.ViewState->TechniqueState_TwoRayGensSubgraph.Var_Enabled = true;

		TwoRayGensSubgraph::AddTechnique(GraphBuilder, View, params);

		// Use any outputs you may want to use
		// params.outputs.
	}

=====================================================================================================
Compile and run
=====================================================================================================

That should be it!

User scoped variables will appear in the console as cvars of the form:

r.TwoRayGensSubgraph.<variable>

Transient and non transient textures are viewable using the "vis" command
and have names in the form of:

TwoRayGensSubgraph.<resource>

See TwoRayGensSubgraphTechnique.cpp for details.
