This package was generated for compatibility with Unreal Engine 5.3.2

=====================================================================================================
Add the Files To Unreal
=====================================================================================================

You can follow the steps below manually, or run install.bat, with the path to the unreal source code
root - the folder where GenerateProjectFiles.bat lives.

Make sure and run GenerateProjectFiles.bat to regenerate the unreal solution to include these new files.

Copy Public/boxblurTechnique.h to Engine/Source/Runtime/Renderer/Public/

Copy Public/boxblurTechnique.cpp to Engine/Source/Runtime/Renderer/Private/

Copy the contents of Shaders to Engine/Shaders/Private/boxblur/

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
  * boxblur_0.usf
  * boxblur_1.usf

=====================================================================================================
Set Up Non Transient Storage
=====================================================================================================

This technique requires non transient storage.

To set that up, put this struct into Engine\Source\Runtime\Renderer\Private\ScenePrivate.h:

struct FTechniqueState_boxblur
{
    // Host Variables - feel free to read and write these
    FVector3f Var_iResolution = FVector3f(0.f, 0.f, 0.f);
    float Var_iTime = float(0.f);
    float Var_iTimeDelta = float(0.f);
    float Var_iFrameRate = float(0.f);
    int32 Var_iFrame = int32(0);
    FVector4f Var_iMouse = FVector4f(0.f, 0.f, 0.f, 0.f);
    FVector4f Var_MouseState = FVector4f(0.f, 0.f, 0.f, 0.f);
    FVector4f Var_MouseStateLastFrame = FVector4f(0.f, 0.f, 0.f, 0.f);

    // User Variables - feel free to read and write these
    bool Var_enabled = bool(true);  // Enables or disables the blurring effect.
    int32 Var_radius = int32(2);  // The radius of the blur.  Actual size in pixles of the blur is (radius*2+1)^2
    bool Var_sRGB = bool(true);
};

and put this member variable into FSceneViewState:

FTechniqueState_boxblur TechniqueState_boxblur;

=====================================================================================================
Connect Technique
=====================================================================================================

The location of where to run the technique, what inputs to give, and what to do with the outputs is
very much specific to the details of the technique and the usage case. However, here is how you
would connect a new post processing effect, as an example.

in Engine/Source/Runtime/Renderer/Private/PostProcess/PostProcessing.cpp:

1) include your header near the top of the file

#include "boxblurTechnique.h"

2) Call into your technique from inside AddPostProcessingPasses()

Around line 318, there will be this line getting the SceneColor texture.

FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, PrimaryViewRect);

Assuming you want your new post processing effect to run first, you can put this chunk of code after that line:

	{
		boxblur::FTechniqueParams params;

		// Set inputs, such as this line:
		//params.inputs.Texture_InputTexture = SceneColor.Texture;

		// Set variables, such as this line:
		// View.ViewState->TechniqueState_boxblur.Var_Enabled = true;

		boxblur::AddTechnique(GraphBuilder, View, params);

		// Use any outputs you may want to use
		// params.outputs.
	}

=====================================================================================================
Compile and run
=====================================================================================================

That should be it!

User scoped variables will appear in the console as cvars of the form:

r.boxblur.<variable>

Transient and non transient textures are viewable using the "vis" command
and have names in the form of:

boxblur.<resource>

See boxblurTechnique.cpp for details.
