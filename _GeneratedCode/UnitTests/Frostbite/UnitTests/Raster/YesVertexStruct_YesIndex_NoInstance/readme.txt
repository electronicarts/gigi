Here are the instructions for how to use this package in Frostbite

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
Put files into frostbite solution
=====================================================================================================

You can follow the steps below manually, or run install.bat, passing the location of the root (the folder that holds the TnT folder).

Copy YesVertexStruct_YesIndex_NoInstanceRenderPassModule.h and YesVertexStruct_YesIndex_NoInstanceShaderPrograms.ddf into Code\Engine\World\Render\Public\Engine.World.Render\YesVertexStruct_YesIndex_NoInstance\

Copy YesVertexStruct_YesIndex_NoInstanceRenderPassModule.cpp and any custom C++ logic files into Code\Engine\World\Render\YesVertexStruct_YesIndex_NoInstance\

Copy YesVertexStruct_YesIndex_NoInstanceShaderProgramDefinitions.cpp and .h into Code\Engine\World\Pipeline\YesVertexStruct_YesIndex_NoInstance\

Copy the contents of the assets folder (if it exists) to the data directory: Raw\YesVertexStruct_YesIndex_NoInstance

Copy the shader files into Code\Engine\World\Render\sourcedata\source\YesVertexStruct_YesIndex_NoInstance\

=====================================================================================================
FB Vars
=====================================================================================================

These user variables can be viewed and modified in drone. Right click drone, select "Show|Hide" and
then click on the "Variable View" tab to interact with them.

YesVertexStruct_YesIndex_NoInstance.viewMode

=====================================================================================================
Regenerate the solution
=====================================================================================================

Regenerate solution

=====================================================================================================
Register the module
=====================================================================================================

In Code\Engine\World\Render\FrameGraphWorldRenderer.cpp, add these lines in the appropriate places:

#include <Engine.World.Render/YesVertexStruct_YesIndex_NoInstance/YesVertexStruct_YesIndex_NoInstanceRenderPassModule.h>

moduleManager.registerModule<YesVertexStruct_YesIndex_NoInstanceRenderPassModule>();

=====================================================================================================
Hook up inputs and outputs
=====================================================================================================

YesVertexStruct_YesIndex_NoInstanceRenderPassModule::addPass() in YesVertexStruct_YesIndex_NoInstanceRenderPassModule.cpp has a todo at the top and
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

=====================================================================================================
Adjust Vertex Shader Semantics
=====================================================================================================

Frostbite only allows TEXCOORD semantics for vertex shader inputs, so make sure these only use those:

Systems/WorldRender/YesVertexStruct_YesIndex_NoInstance/_VSPS_Rasterize.hlsl

=====================================================================================================
Describe vertex format for imported vertex buffers
=====================================================================================================

When a Gigi technique uses an imported buffer for a vertex or instance buffer, it doesn't know what
format the buffer is, so you must describe it manually.

YesVertexStruct_YesIndex_NoInstanceShaderDefinitions.cpp, YesVertexStruct_YesIndex_NoInstanceShaderProgramsProvider::gatherShaderProgramDefinitions():
* Vertex_Buffer needs to be described in program.geometryDeclaration.elements, and stream[0], as VertexElementClassification_PerVertex.

YesVertexStruct_YesIndex_NoInstanceRenderpassModule.cpp, YesVertexStruct_YesIndex_NoInstanceRenderPassModule::addPass():
* bufferVertex_Buffer_vbdesc needs to be filled out as inputSlot 0 and InputClassification_PerVertex.
