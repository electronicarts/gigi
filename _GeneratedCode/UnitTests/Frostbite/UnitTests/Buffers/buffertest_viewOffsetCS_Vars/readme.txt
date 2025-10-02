Here are the instructions for how to use this package in Frostbite

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
Put files into frostbite solution
=====================================================================================================

You can follow the steps below manually, or run install.bat, passing the location of the root (the folder that holds the TnT folder).

Copy buffertest_viewOffsetCS_VarsRenderPassModule.h and buffertest_viewOffsetCS_VarsShaderPrograms.ddf into Code\Engine\World\Render\Public\Engine.World.Render\buffertest_viewOffsetCS_Vars\

Copy buffertest_viewOffsetCS_VarsRenderPassModule.cpp and any custom C++ logic files into Code\Engine\World\Render\buffertest_viewOffsetCS_Vars\

Copy buffertest_viewOffsetCS_VarsShaderProgramDefinitions.cpp and .h into Code\Engine\World\Pipeline\buffertest_viewOffsetCS_Vars\

Copy the contents of the assets folder (if it exists) to the data directory: Raw\buffertest_viewOffsetCS_Vars

Copy the shader files into Code\Engine\World\Render\sourcedata\source\buffertest_viewOffsetCS_Vars\

=====================================================================================================
FB Vars
=====================================================================================================

These user variables can be viewed and modified in drone. Right click drone, select "Show|Hide" and
then click on the "Variable View" tab to interact with them.

Overall Volume Adjustment
buffertest_viewOffsetCS_Vars.gain

Adjusts the contribution of sample n-1
buffertest_viewOffsetCS_Vars.alpha1

Adjusts the contribution of sample n-2
buffertest_viewOffsetCS_Vars.alpha2

=====================================================================================================
Regenerate the solution
=====================================================================================================

Regenerate solution

=====================================================================================================
Register the module
=====================================================================================================

In Code\Engine\World\Render\FrameGraphWorldRenderer.cpp, add these lines in the appropriate places:

#include <Engine.World.Render/buffertest_viewOffsetCS_Vars/buffertest_viewOffsetCS_VarsRenderPassModule.h>

moduleManager.registerModule<buffertest_viewOffsetCS_VarsRenderPassModule>();

=====================================================================================================
Hook up inputs and outputs
=====================================================================================================

buffertest_viewOffsetCS_VarsRenderPassModule::addPass() in buffertest_viewOffsetCS_VarsRenderPassModule.cpp has a todo at the top and
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
