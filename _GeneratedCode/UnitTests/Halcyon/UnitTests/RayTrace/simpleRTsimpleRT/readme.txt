Here are the instructions for how to use this package in Halcyon

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
1) Add this to Common\Inc\Common\Render\RenderGraphPassInterfaces.h
=====================================================================================================

class IsimpleRTPass : public IRenderPass
{
public:
	HCY_DECLARE_INTERFACE(IsimpleRTPass, 0);

	virtual void addsimpleRTPass(RenderGraph& renderGraph, RenderGraphScope& scope, const class ShaderPipelines& pipelines) = 0;
};

=====================================================================================================
2) Add this to Common\Inc\Common\Render\RenderGraphPassList.h
=====================================================================================================

PASS(simpleRT)

=====================================================================================================
3) Add this to Browser\Lib\Renderer.cpp wherever you want the render pass to happen
=====================================================================================================

pass(&IsimpleRTPass::addsimpleRTPass, m_rgsimpleRTPass);

If you are doing a post processing effect, a good place to put this could be right after the "final pass"
which is this line:  pass(&IFinalPass::addFinalPass, m_rgFinalPass);

=====================================================================================================
4) Put simpleRTPass.cpp and simpleRTPass.h and any custom c++ headers or
source files into Modules\Render.Passes\Lib\.
=====================================================================================================

=====================================================================================================
5) Put simpleRT_imgui.h into Browser\Lib.
=====================================================================================================

=====================================================================================================
6) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

//
struct simpleRTPassData
{
	// Set and get these
	RenderGraphResource textureTexture;
	RenderResourceHandle bufferScene;

	// Internal use only
};
DeclareRenderGraphScopeContainer(simpleRTPassData);

=====================================================================================================
7) Hook up the inputs and outputs
=====================================================================================================

// Do this to get access to the pass data whenever you want to read or write it
auto& d = scope.get<simpleRTPassData>();

// Set the input resources
d.bufferScene = /*Buffer Goes Here*/;

// Get and use the results
auto& d = scope.get<simpleRTPassData>();
// use d.bufferScene for whatever you want

If you are doing a post processing effect, a good way to hook up the inputs and outputs is to go into
your simpleRTPass::addsimpleRTPass() function within simpleRTPass.cpp and put lines like these
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
8) Copy the contents of the shaders folder to Shaders\simpleRT\
=====================================================================================================

=====================================================================================================
9) Add the below to Shaders\Shaders.toml.
=====================================================================================================

["Shaders/simpleRT/SimpleRTMiss"]
type = ""
entryPoint = "Miss"
source = "Shaders/simpleRT/simpleRT.hlsl"
flags = ""

["Shaders/simpleRT/SimpleRTClosestHit"]
type = ""
entryPoint = "ClosestHit"
source = "Shaders/simpleRT/simpleRT.hlsl"
flags = ""

["Shaders/simpleRT/DoRT"]
type = ""
entryPoint = "RayGen"
source = "Shaders/simpleRT/simpleRT.hlsl"
flags = ""

=====================================================================================================
10) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

// This goes above struct RenderSettings
struct simpleRTSettings
{
	bool enabled = false;
	glm::mat4 clipToWorld = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
	glm::vec3 cameraPos = {0.f, 0.f, 0.f};
	float depthNearPlane = 0.f;  // The depth value for the near plane.
	glm::vec3 hitColor = {0.f, 1.f, 0.f};
	glm::vec3 missColor = {1.f, 0.f, 0.f};
};

// This goes in the struct RenderSettings
simpleRTSettings  simpleRT;

=====================================================================================================
11) Add this to Browser\Lib\BrowserGui.cpp
=====================================================================================================

// add to the top of the file
#include "simpleRT_imgui.h"

// put this into BrowserGui::doSettingsTweakerGui() wherever you want the UI to go, within the
// render settings section.  You can open the render settings in halcyon by selecting "Render Settings"
// in the Window menu.
simpleRT::MakeUI(m_owner.m_renderSettings.simpleRT);

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


