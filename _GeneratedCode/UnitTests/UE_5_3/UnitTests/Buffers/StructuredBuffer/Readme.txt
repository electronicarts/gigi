This package was generated for compatibility with Unreal Engine 5.3.2

=====================================================================================================
Add the Files To Unreal
=====================================================================================================

You can follow the steps below manually, or run install.bat, with the path to the unreal source code
root - the folder where GenerateProjectFiles.bat lives.

Make sure and run GenerateProjectFiles.bat to regenerate the unreal solution to include these new files.

Copy Public/StructuredBufferTechnique.h to Engine/Source/Runtime/Renderer/Public/

Copy Public/StructuredBufferTechnique.cpp to Engine/Source/Runtime/Renderer/Private/

Copy the contents of Shaders to Engine/Shaders/Private/StructuredBuffer/

=====================================================================================================
Set Up Non Transient Storage
=====================================================================================================

This technique requires non transient storage.

To set that up, put this struct into Engine\Source\Runtime\Renderer\Private\ScenePrivate.h:

struct FTechniqueState_StructuredBuffer
{
    // Host Variables - feel free to read and write these
    int32 Var_frameIndex = int32(0);
    float Var_frameDeltaTime = float(0.f);
};

and put this member variable into FSceneViewState:

FTechniqueState_StructuredBuffer TechniqueState_StructuredBuffer;

=====================================================================================================
Connect Technique
=====================================================================================================

The location of where to run the technique, what inputs to give, and what to do with the outputs is
very much specific to the details of the technique and the usage case. However, here is how you
would connect a new post processing effect, as an example.

in Engine/Source/Runtime/Renderer/Private/PostProcess/PostProcessing.cpp:

1) include your header near the top of the file

#include "StructuredBufferTechnique.h"

2) Call into your technique from inside AddPostProcessingPasses()

Around line 318, there will be this line getting the SceneColor texture.

FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, PrimaryViewRect);

Assuming you want your new post processing effect to run first, you can put this chunk of code after that line:

	{
		StructuredBuffer::FTechniqueParams params;

		// Set inputs, such as this line:
		//params.inputs.Texture_InputTexture = SceneColor.Texture;

		// Set variables, such as this line:
		// View.ViewState->TechniqueState_StructuredBuffer.Var_Enabled = true;

		StructuredBuffer::AddTechnique(GraphBuilder, View, params);

		// Use any outputs you may want to use
		// params.outputs.
	}

=====================================================================================================
Compile and run
=====================================================================================================

That should be it!

User scoped variables will appear in the console as cvars of the form:

r.StructuredBuffer.<variable>

Transient and non transient textures are viewable using the "vis" command
and have names in the form of:

StructuredBuffer.<resource>

See StructuredBufferTechnique.cpp for details.
