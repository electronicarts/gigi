Here are the instructions for how to use this package in Halcyon

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
1) Add this to Common\Inc\Common\Render\RenderGraphPassInterfaces.h
=====================================================================================================

class ISubGraphLoopsPass : public IRenderPass
{
public:
	HCY_DECLARE_INTERFACE(ISubGraphLoopsPass, 0);

	virtual void addSubGraphLoopsPass(RenderGraph& renderGraph, RenderGraphScope& scope, const class ShaderPipelines& pipelines) = 0;
};

=====================================================================================================
2) Add this to Common\Inc\Common\Render\RenderGraphPassList.h
=====================================================================================================

PASS(SubGraphLoops)

=====================================================================================================
3) Add this to Browser\Lib\Renderer.cpp wherever you want the render pass to happen
=====================================================================================================

pass(&ISubGraphLoopsPass::addSubGraphLoopsPass, m_rgSubGraphLoopsPass);

If you are doing a post processing effect, a good place to put this could be right after the "final pass"
which is this line:  pass(&IFinalPass::addFinalPass, m_rgFinalPass);

=====================================================================================================
4) Put SubGraphLoopsPass.cpp and SubGraphLoopsPass.h and any custom c++ headers or
source files into Modules\Render.Passes\Lib\.
=====================================================================================================

=====================================================================================================
5) Put SubGraphLoops_imgui.h into Browser\Lib.
=====================================================================================================

=====================================================================================================
6) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

//
struct SubGraphLoopsPassData
{
	// Set and get these
	RenderGraphResource textureInput;
	glm::uvec3 textureInput_size = {0, 0, 0};
	RenderGraphResource textureFilterSub_Iteration_0_Output;
	RenderGraphResource textureFilterSub_Iteration_1_Output;
	RenderGraphResource textureFilterSub_Iteration_2_Output;
	RenderGraphResource textureFilterSub_Iteration_3_Output;
	RenderGraphResource textureFilterSub_Iteration_4_Output;

	// Internal use only
};
DeclareRenderGraphScopeContainer(SubGraphLoopsPassData);

=====================================================================================================
7) Hook up the inputs and outputs
=====================================================================================================

// Do this to get access to the pass data whenever you want to read or write it
auto& d = scope.get<SubGraphLoopsPassData>();

// Set the input resources
d.textureInput = /*Texture Goes Here*/;
d.textureInput_size = /*Texture Size Goes Here*/;

// Get and use the results
auto& d = scope.get<SubGraphLoopsPassData>();
// use d.textureInput for whatever you want

If you are doing a post processing effect, a good way to hook up the inputs and outputs is to go into
your SubGraphLoopsPass::addSubGraphLoopsPass() function within SubGraphLoopsPass.cpp and put lines like these
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
8) Copy the contents of the shaders folder to Shaders\SubGraphLoops\
=====================================================================================================

=====================================================================================================
9) Add the below to Shaders\Shaders.toml.
=====================================================================================================

["Shaders/SubGraphLoops/FilterSub_Iteration_0_DoBlur"]
type = "cs"
entryPoint = "main"
source = "Shaders/SubGraphLoops/SubGraphLoopsInner_FilterSub Iteration 0/SubGraphLoopsBlur.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

["Shaders/SubGraphLoops/FilterSub_Iteration_1_DoBlur"]
type = "cs"
entryPoint = "main"
source = "Shaders/SubGraphLoops/SubGraphLoopsInner_FilterSub Iteration 1/SubGraphLoopsBlur.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

["Shaders/SubGraphLoops/FilterSub_Iteration_2_DoBlur"]
type = "cs"
entryPoint = "main"
source = "Shaders/SubGraphLoops/SubGraphLoopsInner_FilterSub Iteration 2/SubGraphLoopsBlur.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

["Shaders/SubGraphLoops/FilterSub_Iteration_3_DoBlur"]
type = "cs"
entryPoint = "main"
source = "Shaders/SubGraphLoops/SubGraphLoopsInner_FilterSub Iteration 3/SubGraphLoopsBlur.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

["Shaders/SubGraphLoops/FilterSub_Iteration_4_DoBlur"]
type = "cs"
entryPoint = "main"
source = "Shaders/SubGraphLoops/SubGraphLoopsInner_FilterSub Iteration 4/SubGraphLoopsBlur.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

=====================================================================================================
10) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

// This goes above struct RenderSettings
struct SubGraphLoopsSettings
{
	int FilterSub_Iteration_0_LoopIndex = 0;
	bool FilterSub_Iteration_0_sRGB = true;
	int FilterSub_Iteration_1_LoopIndex = 0;
	bool FilterSub_Iteration_1_sRGB = true;
	int FilterSub_Iteration_2_LoopIndex = 0;
	bool FilterSub_Iteration_2_sRGB = true;
	int FilterSub_Iteration_3_LoopIndex = 0;
	bool FilterSub_Iteration_3_sRGB = true;
	int FilterSub_Iteration_4_LoopIndex = 0;
	bool FilterSub_Iteration_4_sRGB = true;
};

// This goes in the struct RenderSettings
SubGraphLoopsSettings  SubGraphLoops;

=====================================================================================================
11) Add this to Browser\Lib\BrowserGui.cpp
=====================================================================================================

// add to the top of the file
#include "SubGraphLoops_imgui.h"

// put this into BrowserGui::doSettingsTweakerGui() wherever you want the UI to go, within the
// render settings section.  You can open the render settings in halcyon by selecting "Render Settings"
// in the Window menu.
SubGraphLoops::MakeUI(m_owner.m_renderSettings.SubGraphLoops);

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


