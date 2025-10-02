Here are the instructions for how to use this package in Halcyon

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
1) Add this to Common\Inc\Common\Render\RenderGraphPassInterfaces.h
=====================================================================================================

class IprofilingPass : public IRenderPass
{
public:
	HCY_DECLARE_INTERFACE(IprofilingPass, 0);

	virtual void addprofilingPass(RenderGraph& renderGraph, RenderGraphScope& scope, const class ShaderPipelines& pipelines) = 0;
};

=====================================================================================================
2) Add this to Common\Inc\Common\Render\RenderGraphPassList.h
=====================================================================================================

PASS(profiling)

=====================================================================================================
3) Add this to Browser\Lib\Renderer.cpp wherever you want the render pass to happen
=====================================================================================================

pass(&IprofilingPass::addprofilingPass, m_rgprofilingPass);

If you are doing a post processing effect, a good place to put this could be right after the "final pass"
which is this line:  pass(&IFinalPass::addFinalPass, m_rgFinalPass);

=====================================================================================================
4) Put profilingPass.cpp and profilingPass.h and any custom c++ headers or
source files into Modules\Render.Passes\Lib\.
=====================================================================================================

=====================================================================================================
5) Put profiling_imgui.h into Browser\Lib.
=====================================================================================================

=====================================================================================================
6) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

//
struct profilingPassData
{
	// Set and get these
	RenderGraphResource textureInput;
	glm::uvec3 textureInput_size = {0, 0, 0};

	// Internal use only
};
DeclareRenderGraphScopeContainer(profilingPassData);

=====================================================================================================
7) Hook up the inputs and outputs
=====================================================================================================

// Do this to get access to the pass data whenever you want to read or write it
auto& d = scope.get<profilingPassData>();

// Set the input resources
d.textureInput = /*Texture Goes Here*/;
d.textureInput_size = /*Texture Size Goes Here*/;

// Get and use the results
auto& d = scope.get<profilingPassData>();
// use d.textureInput for whatever you want

If you are doing a post processing effect, a good way to hook up the inputs and outputs is to go into
your profilingPass::addprofilingPass() function within profilingPass.cpp and put lines like these
as the first lines of the InitPass lambda.  You will need to adjust these lines for the details of your
technique, but this shows you how to pass the "finalTexture" into your technique, along with the size of
the texture, and also shows how you let the render graph know that you are accessing it for write access
as a UAV for a compute shader.

		auto& finalTexture = scope.get<RenderGraphFinalTexture>();
		finalTexture.finalTexture = build.write(finalTexture.finalTexture, RenderBindFlags::UnorderedAccess);
		passData.textureInputTexture = finalTexture.finalTexture;
		RenderGraphTextureDesc finalTextureDesc;
		build.getTextureDesc(finalTexture.finalTexture, &finalTextureDesc);
		passData.textureInputTexture_size = { finalTextureDesc.width, finalTextureDesc.height, 1 };

=====================================================================================================
8) Copy the contents of the shaders folder to Shaders\profiling\
=====================================================================================================

=====================================================================================================
9) Add the below to Shaders\Shaders.toml.
=====================================================================================================

["Shaders/profiling/DoSimpleCS"]
type = "cs"
entryPoint = "Main"
source = "Shaders/profiling/profiling.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

=====================================================================================================
10) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

// This goes above struct RenderSettings
struct profilingSettings
{
};

// This goes in the struct RenderSettings
profilingSettings  profiling;

=====================================================================================================
11) Add this to Browser\Lib\BrowserGui.cpp
=====================================================================================================

// add to the top of the file
#include "profiling_imgui.h"

// put this into BrowserGui::doSettingsTweakerGui() wherever you want the UI to go, within the
// render settings section.  You can open the render settings in halcyon by selecting "Render Settings"
// in the Window menu.
profiling::MakeUI(m_owner.m_renderSettings.profiling);

=====================================================================================================
12) Deal with the assets folder
=====================================================================================================

TODO: explain what to do!

=====================================================================================================
13) Run Format_Code.bat to make sure the generated code is spaced correctly etc.
=====================================================================================================

=====================================================================================================
14) Run Generate_Win64.bat to regenerate the projects and you are good to go!
=====================================================================================================


