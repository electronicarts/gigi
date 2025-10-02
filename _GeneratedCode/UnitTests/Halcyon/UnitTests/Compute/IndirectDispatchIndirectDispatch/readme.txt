Here are the instructions for how to use this package in Halcyon

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
1) Add this to Common\Inc\Common\Render\RenderGraphPassInterfaces.h
=====================================================================================================

class IIndirectDispatchPass : public IRenderPass
{
public:
	HCY_DECLARE_INTERFACE(IIndirectDispatchPass, 0);

	virtual void addIndirectDispatchPass(RenderGraph& renderGraph, RenderGraphScope& scope, const class ShaderPipelines& pipelines) = 0;
};

=====================================================================================================
2) Add this to Common\Inc\Common\Render\RenderGraphPassList.h
=====================================================================================================

PASS(IndirectDispatch)

=====================================================================================================
3) Add this to Browser\Lib\Renderer.cpp wherever you want the render pass to happen
=====================================================================================================

pass(&IIndirectDispatchPass::addIndirectDispatchPass, m_rgIndirectDispatchPass);

If you are doing a post processing effect, a good place to put this could be right after the "final pass"
which is this line:  pass(&IFinalPass::addFinalPass, m_rgFinalPass);

=====================================================================================================
4) Put IndirectDispatchPass.cpp and IndirectDispatchPass.h and any custom c++ headers or
source files into Modules\Render.Passes\Lib\.
=====================================================================================================

=====================================================================================================
5) Put IndirectDispatch_imgui.h into Browser\Lib.
=====================================================================================================

=====================================================================================================
6) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

//
struct IndirectDispatchPassData
{
	// Set and get these
	RenderGraphResource textureRender_Target;

	// Internal use only
	RenderGraphResource bufferIndirect_Dispatch_Count;
};
DeclareRenderGraphScopeContainer(IndirectDispatchPassData);

=====================================================================================================
7) Hook up the inputs and outputs
=====================================================================================================

// Do this to get access to the pass data whenever you want to read or write it
auto& d = scope.get<IndirectDispatchPassData>();

// Set the input resources

// Get and use the results
auto& d = scope.get<IndirectDispatchPassData>();

If you are doing a post processing effect, a good way to hook up the inputs and outputs is to go into
your IndirectDispatchPass::addIndirectDispatchPass() function within IndirectDispatchPass.cpp and put lines like these
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
8) Copy the contents of the shaders folder to Shaders\IndirectDispatch\
=====================================================================================================

=====================================================================================================
9) Add the below to Shaders\Shaders.toml.
=====================================================================================================

["Shaders/IndirectDispatch/Clear_Render_Target"]
type = "cs"
entryPoint = "csmain"
source = "Shaders/IndirectDispatch/IndirectDispatch_Clear.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

["Shaders/IndirectDispatch/Fill_Indirect_Dispatch_Count"]
type = "cs"
entryPoint = "FillIndirectDispatchCount"
source = "Shaders/IndirectDispatch/IndirectDispatch_Fill.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

["Shaders/IndirectDispatch/Do_Indirect_Dispatch_1"]
type = "cs"
entryPoint = "DoIndirectDispatch"
source = "Shaders/IndirectDispatch/IndirectDispatch_Execute.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

["Shaders/IndirectDispatch/Do_Indirect_Dispatch_2"]
type = "cs"
entryPoint = "DoIndirectDispatch"
source = "Shaders/IndirectDispatch/IndirectDispatch_Execute.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

=====================================================================================================
10) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

// This goes above struct RenderSettings
struct IndirectDispatchSettings
{
	glm::uvec3 Dispatch_Count_1 = {50, 50, 1};
	glm::uvec3 Dispatch_Count_2 = {100, 100, 1};
	int Second_Dispatch_Offset = 2;
};

// This goes in the struct RenderSettings
IndirectDispatchSettings  IndirectDispatch;

=====================================================================================================
11) Add this to Browser\Lib\BrowserGui.cpp
=====================================================================================================

// add to the top of the file
#include "IndirectDispatch_imgui.h"

// put this into BrowserGui::doSettingsTweakerGui() wherever you want the UI to go, within the
// render settings section.  You can open the render settings in halcyon by selecting "Render Settings"
// in the Window menu.
IndirectDispatch::MakeUI(m_owner.m_renderSettings.IndirectDispatch);

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


