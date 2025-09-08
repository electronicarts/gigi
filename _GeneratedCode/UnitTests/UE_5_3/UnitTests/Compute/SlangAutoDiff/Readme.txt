This package was generated for compatibility with Unreal Engine 5.3.2

=====================================================================================================
Add the Files To Unreal
=====================================================================================================

You can follow the steps below manually, or run install.bat, with the path to the unreal source code
root - the folder where GenerateProjectFiles.bat lives.

Make sure and run GenerateProjectFiles.bat to regenerate the unreal solution to include these new files.

Copy Public/SlangAutoDiffTechnique.h to Engine/Source/Runtime/Renderer/Public/

Copy Public/SlangAutoDiffTechnique.cpp to Engine/Source/Runtime/Renderer/Private/

Copy the contents of Shaders to Engine/Shaders/Private/SlangAutoDiff/

=====================================================================================================
Slang Warning
=====================================================================================================

Some shader files were processed by slang and may need some manual fixup. Slang sometimes
renames resources by appending a 0, such as "Data" becoming "Data_0". Slang also adds register
declarations onto the resources, and puts globally scoped uniform values into a struct.  These things
will make shader compilation errors because the shader does not match the C++ definition of params.

To fix it, you can remove the register declarations, put the resource names back to what they should be
and you can take the uniform values out of the struct.  To satisfy the slang code that references those
things that you just un-renamed, you can use #defines to make them still accessible in the slang code.

For instance if slang made it look like this:

RWBuffer<float > Data_0 : register(u0);

Change that to this:

RWBuffer<float> Data;
#define Data_0 Data

For the constants in structs, you'll need to replace the . of the struct with an underscore or similar.

Shaders Files Affected:
  * SlangAutoDiff_Descend_0.usf

=====================================================================================================
Set Up Non Transient Storage
=====================================================================================================

This technique requires non transient storage.

To set that up, put this struct into Engine\Source\Runtime\Renderer\Private\ScenePrivate.h:

struct FTechniqueState_SlangAutoDiff
{
    TRefCountPtr<FRDGPooledBuffer> Buffer_Data;  // first 2 floats is position, nect 2 floats are the gradient. Then 5 floats per Gaussian.

    // Host Variables - feel free to read and write these
    bool Var_initialized = bool(false);
    int32 Var_FrameIndex = int32(0);
    FVector4f Var_MouseState = FVector4f(0.f, 0.f, 0.f, 0.f);
    FVector4f Var_MouseStateLastFrame = FVector4f(0.f, 0.f, 0.f, 0.f);
    FVector3f Var_iResolution = FVector3f(0.f, 0.f, 0.f);

    // User Variables - feel free to read and write these
    int32 Var_NumGaussians = int32(10);
    bool Var_Reset = bool(false);
    float Var_LearningRate = float(0.100000001f);
    float Var_MaximumStepSize = float(0.00999999978f);
    bool Var_UseBackwardAD = bool(true);
    bool Var_QuantizeDisplay = bool(false);
};

and put this member variable into FSceneViewState:

FTechniqueState_SlangAutoDiff TechniqueState_SlangAutoDiff;

=====================================================================================================
Connect Technique
=====================================================================================================

The location of where to run the technique, what inputs to give, and what to do with the outputs is
very much specific to the details of the technique and the usage case. However, here is how you
would connect a new post processing effect, as an example.

in Engine/Source/Runtime/Renderer/Private/PostProcess/PostProcessing.cpp:

1) include your header near the top of the file

#include "SlangAutoDiffTechnique.h"

2) Call into your technique from inside AddPostProcessingPasses()

Around line 318, there will be this line getting the SceneColor texture.

FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, PrimaryViewRect);

Assuming you want your new post processing effect to run first, you can put this chunk of code after that line:

	{
		SlangAutoDiff::FTechniqueParams params;

		// Set inputs, such as this line:
		//params.inputs.Texture_InputTexture = SceneColor.Texture;

		// Set variables, such as this line:
		// View.ViewState->TechniqueState_SlangAutoDiff.Var_Enabled = true;

		SlangAutoDiff::AddTechnique(GraphBuilder, View, params);

		// Use any outputs you may want to use
		// params.outputs.
	}

=====================================================================================================
Compile and run
=====================================================================================================

That should be it!

User scoped variables will appear in the console as cvars of the form:

r.SlangAutoDiff.<variable>

Transient and non transient textures are viewable using the "vis" command
and have names in the form of:

SlangAutoDiff.<resource>

See SlangAutoDiffTechnique.cpp for details.
