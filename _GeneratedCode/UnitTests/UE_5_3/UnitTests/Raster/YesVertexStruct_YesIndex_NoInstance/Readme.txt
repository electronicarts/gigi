This package was generated for compatibility with Unreal Engine 5.3.2

=====================================================================================================
Add the Files To Unreal
=====================================================================================================

You can follow the steps below manually, or run install.bat, with the path to the unreal source code
root - the folder where GenerateProjectFiles.bat lives.

Make sure and run GenerateProjectFiles.bat to regenerate the unreal solution to include these new files.

Copy Public/YesVertexStruct_YesIndex_NoInstanceTechnique.h to Engine/Source/Runtime/Renderer/Public/

Copy Public/YesVertexStruct_YesIndex_NoInstanceTechnique.cpp to Engine/Source/Runtime/Renderer/Private/

Copy the contents of Shaders to Engine/Shaders/Private/YesVertexStruct_YesIndex_NoInstance/

=====================================================================================================
Modify Vertex Buffer Semantics in Shaders
=====================================================================================================

If you use semantics like POSITION, NORMAL, TEXCOORD in the inputs to a vertex shader, you need to
update those to use ATTRIBUTE[N] semantics instead.

For instance:

 struct VSInput
 {
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
 };

 Should become:

  struct VSInput
 {
	float3 position : ATTRIBUTE0;
	float3 normal : ATTRIBUTE1;
	float2 uv : ATTRIBUTE2;
 };

Shaders Files Potentially Affected:
  * YesVertexStruct_YesIndex_NoInstance_VS.usf

=====================================================================================================
Set Up Non Transient Storage
=====================================================================================================

This technique requires non transient storage.

To set that up, put this struct into Engine\Source\Runtime\Renderer\Private\ScenePrivate.h:

struct FTechniqueState_YesVertexStruct_YesIndex_NoInstance
{
    // Host Variables - feel free to read and write these
    FVector4f Var_MouseState = FVector4f(0.f, 0.f, 0.f, 0.f);
    FVector4f Var_MouseStateLastFrame = FVector4f(0.f, 0.f, 0.f, 0.f);
    FVector3f Var_iResolution = FVector3f(0.f, 0.f, 0.f);
    float Var_iTime = float(0.f);
    float Var_iTimeDelta = float(0.f);
    float Var_iFrameRate = float(0.f);
    int32 Var_iFrame = int32(0);
    FVector4f Var_iMouse = FVector4f(0.f, 0.f, 0.f, 0.f);
    FMatrix44f Var_ViewMtx = FMatrix44f(FPlane4f(1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f), FPlane4f(0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f), FPlane4f(0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f), FPlane4f(0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f));
    FMatrix44f Var_InvViewMtx = FMatrix44f(FPlane4f(1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f), FPlane4f(0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f), FPlane4f(0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f), FPlane4f(0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f));
    FMatrix44f Var_ProjMtx = FMatrix44f(FPlane4f(1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f), FPlane4f(0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f), FPlane4f(0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f), FPlane4f(0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f));
    FMatrix44f Var_InvProjMtx = FMatrix44f(FPlane4f(1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f), FPlane4f(0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f), FPlane4f(0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f), FPlane4f(0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f));
    FMatrix44f Var_ViewProjMtx = FMatrix44f(FPlane4f(1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f), FPlane4f(0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f), FPlane4f(0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f), FPlane4f(0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f));
    FMatrix44f Var_InvViewProjMtx = FMatrix44f(FPlane4f(1.000000000f, 0.000000000f, 0.000000000f, 0.000000000f), FPlane4f(0.000000000f, 1.000000000f, 0.000000000f, 0.000000000f), FPlane4f(0.000000000f, 0.000000000f, 1.000000000f, 0.000000000f), FPlane4f(0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f));
    FVector3f Var_CameraPos = FVector3f(0.f, 0.f, 0.f);

    // User Variables - feel free to read and write these
    int32 Var_viewMode = int32(0);
};

and put this member variable into FSceneViewState:

FTechniqueState_YesVertexStruct_YesIndex_NoInstance TechniqueState_YesVertexStruct_YesIndex_NoInstance;

=====================================================================================================
Connect Technique
=====================================================================================================

The location of where to run the technique, what inputs to give, and what to do with the outputs is
very much specific to the details of the technique and the usage case. However, here is how you
would connect a new post processing effect, as an example.

in Engine/Source/Runtime/Renderer/Private/PostProcess/PostProcessing.cpp:

1) include your header near the top of the file

#include "YesVertexStruct_YesIndex_NoInstanceTechnique.h"

2) Call into your technique from inside AddPostProcessingPasses()

Around line 318, there will be this line getting the SceneColor texture.

FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, PrimaryViewRect);

Assuming you want your new post processing effect to run first, you can put this chunk of code after that line:

	{
		YesVertexStruct_YesIndex_NoInstance::FTechniqueParams params;

		// Set inputs, such as this line:
		//params.inputs.Texture_InputTexture = SceneColor.Texture;

		// Set variables, such as this line:
		// View.ViewState->TechniqueState_YesVertexStruct_YesIndex_NoInstance.Var_Enabled = true;

		YesVertexStruct_YesIndex_NoInstance::AddTechnique(GraphBuilder, View, params);

		// Use any outputs you may want to use
		// params.outputs.
	}

=====================================================================================================
Compile and run
=====================================================================================================

That should be it!

User scoped variables will appear in the console as cvars of the form:

r.YesVertexStruct_YesIndex_NoInstance.<variable>

Transient and non transient textures are viewable using the "vis" command
and have names in the form of:

YesVertexStruct_YesIndex_NoInstance.<resource>

See YesVertexStruct_YesIndex_NoInstanceTechnique.cpp for details.
