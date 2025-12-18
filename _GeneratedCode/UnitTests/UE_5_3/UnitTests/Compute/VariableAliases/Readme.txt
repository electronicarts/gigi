This package was generated for compatibility with Unreal Engine 5.3.2

=====================================================================================================
Add the Files To Unreal
=====================================================================================================

You can follow the steps below manually, or run install.bat, with the path to the unreal source code
root - the folder where GenerateProjectFiles.bat lives.

Make sure and run GenerateProjectFiles.bat to regenerate the unreal solution to include these new files.

Copy Public/VariableAliasesTechnique.h to Engine/Source/Runtime/Renderer/Public/

Copy Public/VariableAliasesTechnique.cpp to Engine/Source/Runtime/Renderer/Private/

Copy the contents of Shaders to Engine/Shaders/Private/VariableAliases/

=====================================================================================================
Set Up Non Transient Storage
=====================================================================================================

This technique requires non transient storage.

To set that up, put this struct into Engine\Source\Runtime\Renderer\Private\ScenePrivate.h:

struct FTechniqueState_VariableAliases
{
    // Host Variables - feel free to read and write these
    FUintVector2 Var_RenderSize = FUintVector2(32, 32);
    float Var_ConstMultiplier = float(1.f);

    // User Variables - feel free to read and write these
    int32 Var_Node1Channel = int32(0);
    float Var_Node1Value = float(0.25f);
    int32 Var_Node2Channel = int32(1);
    float Var_Node2Value = float(0.5f);
    int32 Var_Node3Channel = int32(2);
    float Var_Node3Value = float(0.75f);
    float Var_UnusedFloat = float(0.f);  // This is for the unused alias in the shader
};

and put this member variable into FSceneViewState:

FTechniqueState_VariableAliases TechniqueState_VariableAliases;

=====================================================================================================
Connect Technique
=====================================================================================================

The location of where to run the technique, what inputs to give, and what to do with the outputs is
very much specific to the details of the technique and the usage case. However, here is how you
would connect a new post processing effect, as an example.

in Engine/Source/Runtime/Renderer/Private/PostProcess/PostProcessing.cpp:

1) include your header near the top of the file

#include "VariableAliasesTechnique.h"

2) Call into your technique from inside AddPostProcessingPasses()

Around line 318, there will be this line getting the SceneColor texture.

FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, PrimaryViewRect);

Assuming you want your new post processing effect to run first, you can put this chunk of code after that line:

	{
		VariableAliases::FTechniqueParams params;

		// Set inputs, such as this line:
		//params.inputs.Texture_InputTexture = SceneColor.Texture;

		// Set variables, such as this line:
		// View.ViewState->TechniqueState_VariableAliases.Var_Enabled = true;

		VariableAliases::AddTechnique(GraphBuilder, View, params);

		// Use any outputs you may want to use
		// params.outputs.
	}

=====================================================================================================
Compile and run
=====================================================================================================

That should be it!

User scoped variables will appear in the console as cvars of the form:

r.VariableAliases.<variable>

Transient and non transient textures are viewable using the "vis" command
and have names in the form of:

VariableAliases.<resource>

See VariableAliasesTechnique.cpp for details.
