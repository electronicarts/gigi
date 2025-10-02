Here are the instructions for how to use this package in Halcyon

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
1) Add this to Common\Inc\Common\Render\RenderGraphPassInterfaces.h
=====================================================================================================

class ITexture2DRW_CSPass : public IRenderPass
{
public:
	HCY_DECLARE_INTERFACE(ITexture2DRW_CSPass, 0);

	virtual void addTexture2DRW_CSPass(RenderGraph& renderGraph, RenderGraphScope& scope, const class ShaderPipelines& pipelines) = 0;
};

=====================================================================================================
2) Add this to Common\Inc\Common\Render\RenderGraphPassList.h
=====================================================================================================

PASS(Texture2DRW_CS)

=====================================================================================================
3) Add this to Browser\Lib\Renderer.cpp wherever you want the render pass to happen
=====================================================================================================

pass(&ITexture2DRW_CSPass::addTexture2DRW_CSPass, m_rgTexture2DRW_CSPass);

If you are doing a post processing effect, a good place to put this could be right after the "final pass"
which is this line:  pass(&IFinalPass::addFinalPass, m_rgFinalPass);

=====================================================================================================
4) Put Texture2DRW_CSPass.cpp and Texture2DRW_CSPass.h and any custom c++ headers or
source files into Modules\Render.Passes\Lib\.
=====================================================================================================

=====================================================================================================
5) Put Texture2DRW_CS_imgui.h into Browser\Lib.
=====================================================================================================

=====================================================================================================
6) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

//
struct Texture2DRW_CSPassData
{
	// Set and get these
	RenderGraphResource textureNodeTexture;
	RenderGraphResource textureImportedTexture;
	glm::uvec3 textureImportedTexture_size = {0, 0, 0};
	RenderGraphResource textureImportedColor;
	glm::uvec3 textureImportedColor_size = {0, 0, 0};

	// Internal use only
	RenderGraphResource texture_loadedTexture_0;
};
DeclareRenderGraphScopeContainer(Texture2DRW_CSPassData);

=====================================================================================================
7) Hook up the inputs and outputs
=====================================================================================================

// Do this to get access to the pass data whenever you want to read or write it
auto& d = scope.get<Texture2DRW_CSPassData>();

// Set the input resources
d.textureImportedTexture = /*Texture Goes Here*/;
d.textureImportedTexture_size = /*Texture Size Goes Here*/;
d.textureImportedColor = /*Texture Goes Here*/;
d.textureImportedColor_size = /*Texture Size Goes Here*/;

// Get and use the results
auto& d = scope.get<Texture2DRW_CSPassData>();
// use d.textureImportedTexture for whatever you want
// use d.textureImportedColor for whatever you want

If you are doing a post processing effect, a good way to hook up the inputs and outputs is to go into
your Texture2DRW_CSPass::addTexture2DRW_CSPass() function within Texture2DRW_CSPass.cpp and put lines like these
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
8) Copy the contents of the shaders folder to Shaders\Texture2DRW_CS\
=====================================================================================================

=====================================================================================================
9) Add the below to Shaders\Shaders.toml.
=====================================================================================================

["Shaders/Texture2DRW_CS/RW"]
type = "cs"
entryPoint = "csmain"
source = "Shaders/Texture2DRW_CS/Texture2DRW_CS.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

=====================================================================================================
10) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

// This goes above struct RenderSettings
struct Texture2DRW_CSSettings
{
};

// This goes in the struct RenderSettings
Texture2DRW_CSSettings  Texture2DRW_CS;

=====================================================================================================
11) Add this to Browser\Lib\BrowserGui.cpp
=====================================================================================================

// add to the top of the file
#include "Texture2DRW_CS_imgui.h"

// put this into BrowserGui::doSettingsTweakerGui() wherever you want the UI to go, within the
// render settings section.  You can open the render settings in halcyon by selecting "Render Settings"
// in the Window menu.
Texture2DRW_CS::MakeUI(m_owner.m_renderSettings.Texture2DRW_CS);

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


