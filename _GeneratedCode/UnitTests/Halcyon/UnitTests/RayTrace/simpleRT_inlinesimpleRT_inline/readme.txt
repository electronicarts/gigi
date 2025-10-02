Here are the instructions for how to use this package in Halcyon

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
1) Add this to Common\Inc\Common\Render\RenderGraphPassInterfaces.h
=====================================================================================================

class IsimpleRT_inlinePass : public IRenderPass
{
public:
	HCY_DECLARE_INTERFACE(IsimpleRT_inlinePass, 0);

	virtual void addsimpleRT_inlinePass(RenderGraph& renderGraph, RenderGraphScope& scope, const class ShaderPipelines& pipelines) = 0;
};

=====================================================================================================
2) Add this to Common\Inc\Common\Render\RenderGraphPassList.h
=====================================================================================================

PASS(simpleRT_inline)

=====================================================================================================
3) Add this to Browser\Lib\Renderer.cpp wherever you want the render pass to happen
=====================================================================================================

pass(&IsimpleRT_inlinePass::addsimpleRT_inlinePass, m_rgsimpleRT_inlinePass);

If you are doing a post processing effect, a good place to put this could be right after the "final pass"
which is this line:  pass(&IFinalPass::addFinalPass, m_rgFinalPass);

=====================================================================================================
4) Put simpleRT_inlinePass.cpp and simpleRT_inlinePass.h and any custom c++ headers or
source files into Modules\Render.Passes\Lib\.
=====================================================================================================

=====================================================================================================
5) Put simpleRT_inline_imgui.h into Browser\Lib.
=====================================================================================================

=====================================================================================================
6) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

//
struct simpleRT_inlinePassData
{
	// Set and get these
	RenderGraphResource textureTexture;
	RenderResourceHandle bufferScene;
	RenderGraphResource bufferScene_VB;

	// Internal use only
};
DeclareRenderGraphScopeContainer(simpleRT_inlinePassData);

=====================================================================================================
7) Hook up the inputs and outputs
=====================================================================================================

// Do this to get access to the pass data whenever you want to read or write it
auto& d = scope.get<simpleRT_inlinePassData>();

// Set the input resources
d.bufferScene = /*Buffer Goes Here*/;
d.bufferScene_VB = /*Buffer Goes Here*/;

// Get and use the results
auto& d = scope.get<simpleRT_inlinePassData>();
// use d.bufferScene for whatever you want
// use d.bufferScene_VB for whatever you want

If you are doing a post processing effect, a good way to hook up the inputs and outputs is to go into
your simpleRT_inlinePass::addsimpleRT_inlinePass() function within simpleRT_inlinePass.cpp and put lines like these
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
8) Copy the contents of the shaders folder to Shaders\simpleRT_inline\
=====================================================================================================

=====================================================================================================
9) Add the below to Shaders\Shaders.toml.
=====================================================================================================

["Shaders/simpleRT_inline/DoRT"]
type = "cs"
entryPoint = "SimpleRTCS"
source = "Shaders/simpleRT_inline/simpleRT_inline.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

=====================================================================================================
10) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

// This goes above struct RenderSettings
struct simpleRT_inlineSettings
{
	bool enabled = false;
	glm::mat4 clipToWorld = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	glm::vec3 cameraPos = {0.f, 0.f, 0.f};
	float depthNearPlane = 0.f;  // The depth value for the near plane.
	glm::vec3 hitColor = {0.f, 1.f, 0.f};
	glm::vec3 missColor = {1.f, 0.f, 0.f};
};

// This goes in the struct RenderSettings
simpleRT_inlineSettings  simpleRT_inline;

=====================================================================================================
11) Add this to Browser\Lib\BrowserGui.cpp
=====================================================================================================

// add to the top of the file
#include "simpleRT_inline_imgui.h"

// put this into BrowserGui::doSettingsTweakerGui() wherever you want the UI to go, within the
// render settings section.  You can open the render settings in halcyon by selecting "Render Settings"
// in the Window menu.
simpleRT_inline::MakeUI(m_owner.m_renderSettings.simpleRT_inline);

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


