This package was generated for compatibility with Unreal Engine 5.3.2

=====================================================================================================
Add the Files To Unreal
=====================================================================================================

You can follow the steps below manually, or run install.bat, with the path to the unreal source code
root - the folder where GenerateProjectFiles.bat lives.

Make sure and run GenerateProjectFiles.bat to regenerate the unreal solution to include these new files.

Copy Public/StencilTechnique.h to Engine/Source/Runtime/Renderer/Public/

Copy Public/StencilTechnique.cpp to Engine/Source/Runtime/Renderer/Private/

Copy the contents of Shaders to Engine/Shaders/Private/Stencil/

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
  * StencilDraw1VS.usf
  * StencilDraw2VS.usf

=====================================================================================================
Asset Setup
=====================================================================================================

This technique requires additional setup for required assets.

These assets must be imported into the engine:

assets/cabinsmall.png (sRGB: false)

Stencil::Initialize() needs to be called from the game thread to load the textures for use on the rendering thread.

A good place to do that could be in SceneRendering.cpp. Near the top, add an include to your technique:

#include "StencilTechnique.h"

Then, add this into the FSceneRenderer constructor, right below where LoadBlueNoiseTexture() is called, near line 2568.

        {
            Stencil::FInitParams initParams;
            // TODO: set path to imported assets.
            // example: initParams.TexturePath__loadedTexture_0 = FSoftObjectPath("/Engine/EngineMaterials/cabinsmall.cabinsmall");
            Stencil::Initialize(initParams);
        }

=====================================================================================================
Set Up Non Transient Storage
=====================================================================================================

This technique requires non transient storage.

To set that up, put this struct into Engine\Source\Runtime\Renderer\Private\ScenePrivate.h:

struct FTechniqueState_Stencil
{
    FTextureRHIRef Texture_Depth_Stencil = nullptr;

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
    bool Var_CameraChanged = bool(false);
};

and put this member variable into FSceneViewState:

FTechniqueState_Stencil TechniqueState_Stencil;

=====================================================================================================
Connect Technique
=====================================================================================================

The location of where to run the technique, what inputs to give, and what to do with the outputs is
very much specific to the details of the technique and the usage case. However, here is how you
would connect a new post processing effect, as an example.

in Engine/Source/Runtime/Renderer/Private/PostProcess/PostProcessing.cpp:

1) include your header near the top of the file

#include "StencilTechnique.h"

2) Call into your technique from inside AddPostProcessingPasses()

Around line 318, there will be this line getting the SceneColor texture.

FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, PrimaryViewRect);

Assuming you want your new post processing effect to run first, you can put this chunk of code after that line:

	{
		Stencil::FTechniqueParams params;

		// Set inputs, such as this line:
		//params.inputs.Texture_InputTexture = SceneColor.Texture;

		// Set variables, such as this line:
		// View.ViewState->TechniqueState_Stencil.Var_Enabled = true;

		Stencil::AddTechnique(GraphBuilder, View, params);

		// Use any outputs you may want to use
		// params.outputs.
	}

=====================================================================================================
Compile and run
=====================================================================================================

That should be it!

User scoped variables will appear in the console as cvars of the form:

r.Stencil.<variable>

Transient and non transient textures are viewable using the "vis" command
and have names in the form of:

Stencil.<resource>

See StencilTechnique.cpp for details.
