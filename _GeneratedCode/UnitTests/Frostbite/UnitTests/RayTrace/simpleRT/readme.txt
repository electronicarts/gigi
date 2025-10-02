Here are the instructions for how to use this package in Frostbite

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
Put files into frostbite solution
=====================================================================================================

You can follow the steps below manually, or run install.bat, passing the location of the root (the folder that holds the TnT folder).

Copy simpleRTRenderPassModule.h and simpleRTShaderPrograms.ddf into Code\Engine\World\Render\Public\Engine.World.Render\simpleRT\

Copy simpleRTRenderPassModule.cpp and any custom C++ logic files into Code\Engine\World\Render\simpleRT\

Copy simpleRTShaderProgramDefinitions.cpp and .h into Code\Engine\World\Pipeline\simpleRT\

Copy the contents of the assets folder (if it exists) to the data directory: Raw\simpleRT

Copy the shader files into Code\Engine\World\Render\sourcedata\source\simpleRT\

=====================================================================================================
Enable Raytracing
=====================================================================================================

Add this to local.cfg (such as in DataSets\ExampleData\Scripts\):

Render.RaytraceEnable true

=====================================================================================================
Add The Raytracing Shader Programs
=====================================================================================================

Code/Engine/World/Render/Public/Engine.World.Render/Raytrace/RaytraceShaderPrograms.ddf

Add these lines:
	// simpleRT Shaders
	ShaderProgram_simpleRT_SimpleRTRayGen_RG,
	ShaderProgram_simpleRT_SimpleRTMiss_MS,
	ShaderProgram_simpleRT_SimpleRTClosestHit_HG,

=====================================================================================================
Add The Raytracing Worlds
=====================================================================================================

A ray tracing world need to be created for each closest hit shader used by this technique. Creating a
world involves adding a value to the RaytraceWorldType enum, and then handling all the places that
enum is looked at. The relevant code seems to be under active development still so it seems best to do
a find in files for an existing world type and update the places you find

Add these to RaytraceWorldType in Code/Engine/Render/Public/Engine.Render/Raytrace/RaytraceWorldTypes.h:

	RaytraceWorldType_simpleRT_DoRT_World,

=====================================================================================================
Raytracing Notes
=====================================================================================================

Due to threading limitations, each RtTrace object can only be dispatched once per frame, but the addPass function
is called multiple times in a frame for each view.  The generated code handles this by only rendering for
the main / root view, which avoids the assert, but limits the capabilities of the technique.  If this becomes
a problem, the generated code could be modified to support multiple views.

=====================================================================================================
FB Vars
=====================================================================================================

These user variables can be viewed and modified in drone. Right click drone, select "Show|Hide" and
then click on the "Variable View" tab to interact with them.

simpleRT.enabled

simpleRT.hitColor

simpleRT.missColor

=====================================================================================================
Regenerate the solution
=====================================================================================================

Regenerate solution

=====================================================================================================
Register the module
=====================================================================================================

In Code\Engine\World\Render\FrameGraphWorldRenderer.cpp, add these lines in the appropriate places:

#include <Engine.World.Render/simpleRT/simpleRTRenderPassModule.h>

moduleManager.registerModule<simpleRTRenderPassModule>();

=====================================================================================================
Hook up inputs and outputs
=====================================================================================================

simpleRTRenderPassModule::addPass() in simpleRTRenderPassModule.cpp has a todo at the top and
bottom for you to hook up the inputs and outputs of the techniques.  These will only show up if you
have imported or exported textures that were not connected through the blackboard links in the editor.

For some hints on how to hook a techniue up, if you want to add lighting to a scene, use bookmark
RenderPassBookmark_PreLightTilePass, and this will give you the indirect lighting buffer to write to:

	auto& gbuffer = paramIO.gbuffer;
	auto indirectBufferIndex = gbuffer.description.getBufferIndex(GbufferDescription::Parameter_IndirectLighting);
	FrameGraphMutableResource& textureTexture = gbuffer.gbufferTextures[indirectBufferIndex];

If you have a post effect that you want to apply later in the pipeline, you might use the bookmark
RenderPassBookmark_AfterPost and use this texture instead:

blackboard.get<ViewBlackboardData>().colorTexture

Note that you can also assign that colorTexture to a texture if you want to at the end of the technique:

blackboard.get<ViewBlackboardData>().colorTexture = myTexture;

It's also common to want the frame index, as that might be used to cycle a texture, or to generate
per frame random numbers.  You can get that at:

blackboard.get<ViewBlackboardData>().frameIndex;

Sometimes while debugging, your render passes will write to things not used by anything else, causing
Frostbite to cull the passes and not run them.  You can force a pass to be ran by calling
builder.registerSideEffect().
