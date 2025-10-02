Here are the instructions for how to use this package in Halcyon

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
1) Add this to Common\Inc\Common\Render\RenderGraphPassInterfaces.h
=====================================================================================================

class IboxblurPass : public IRenderPass
{
public:
	HCY_DECLARE_INTERFACE(IboxblurPass, 0);

	virtual void addboxblurPass(RenderGraph& renderGraph, RenderGraphScope& scope, const class ShaderPipelines& pipelines) = 0;
};

=====================================================================================================
2) Add this to Common\Inc\Common\Render\RenderGraphPassList.h
=====================================================================================================

PASS(boxblur)

=====================================================================================================
3) Add this to Browser\Lib\Renderer.cpp wherever you want the render pass to happen
=====================================================================================================

pass(&IboxblurPass::addboxblurPass, m_rgboxblurPass);

If you are doing a post processing effect, a good place to put this could be right after the "final pass"
which is this line:  pass(&IFinalPass::addFinalPass, m_rgFinalPass);

=====================================================================================================
4) Put boxblurPass.cpp and boxblurPass.h and any custom c++ headers or
source files into Modules\Render.Passes\Lib\.
=====================================================================================================

=====================================================================================================
5) Put boxblur_imgui.h into Browser\Lib.
=====================================================================================================

=====================================================================================================
6) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

//
struct boxblurPassData
{
	// Set and get these
	RenderGraphResource textureInputTexture;
	glm::uvec3 textureInputTexture_size = {0, 0, 0};

	// Internal use only
	RenderGraphResource texturePingPongTexture;
};
DeclareRenderGraphScopeContainer(boxblurPassData);

=====================================================================================================
7) Hook up the inputs and outputs
=====================================================================================================

// Do this to get access to the pass data whenever you want to read or write it
auto& d = scope.get<boxblurPassData>();

// Set the input resources
d.textureInputTexture = /*Texture Goes Here*/;
d.textureInputTexture_size = /*Texture Size Goes Here*/;

// Get and use the results
auto& d = scope.get<boxblurPassData>();
// use d.textureInputTexture for whatever you want

If you are doing a post processing effect, a good way to hook up the inputs and outputs is to go into
your boxblurPass::addboxblurPass() function within boxblurPass.cpp and put lines like these
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
8) Copy the contents of the shaders folder to Shaders\boxblur\
=====================================================================================================

=====================================================================================================
9) Add the below to Shaders\Shaders.toml.
=====================================================================================================

["Shaders/boxblur/BlurH"]
type = "cs"
entryPoint = "BlurH"
source = "Shaders/boxblur/boxblur_0.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

["Shaders/boxblur/BlurV"]
type = "cs"
entryPoint = "BlurV"
source = "Shaders/boxblur/boxblur_1.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

=====================================================================================================
10) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

// This goes above struct RenderSettings
struct boxblurSettings
{
	bool enabled = true;  // Enables or disables the blurring effect.
	int radius = 2;  // The radius of the blur.  Actual size in pixles of the blur is (radius*2+1)^2
	bool sRGB = true;
	glm::vec3 iResolution = {0.f, 0.f, 0.f};
	float iTime = 0.f;
	float iTimeDelta = 0.f;
	float iFrameRate = 0.f;
	int iFrame = 0;
	glm::vec4 iMouse = {0.f, 0.f, 0.f, 0.f};
	glm::vec4 MouseState = {0.f, 0.f, 0.f, 0.f};
	glm::vec4 MouseStateLastFrame = {0.f, 0.f, 0.f, 0.f};
};

// This goes in the struct RenderSettings
boxblurSettings  boxblur;

=====================================================================================================
11) Add this to Browser\Lib\BrowserGui.cpp
=====================================================================================================

// add to the top of the file
#include "boxblur_imgui.h"

// put this into BrowserGui::doSettingsTweakerGui() wherever you want the UI to go, within the
// render settings section.  You can open the render settings in halcyon by selecting "Render Settings"
// in the Window menu.
boxblur::MakeUI(m_owner.m_renderSettings.boxblur);

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


