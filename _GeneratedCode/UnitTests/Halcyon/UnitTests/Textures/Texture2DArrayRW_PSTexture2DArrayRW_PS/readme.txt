Here are the instructions for how to use this package in Halcyon

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
1) Add this to Common\Inc\Common\Render\RenderGraphPassInterfaces.h
=====================================================================================================

class ITexture2DArrayRW_PSPass : public IRenderPass
{
public:
	HCY_DECLARE_INTERFACE(ITexture2DArrayRW_PSPass, 0);

	virtual void addTexture2DArrayRW_PSPass(RenderGraph& renderGraph, RenderGraphScope& scope, const class ShaderPipelines& pipelines) = 0;
};

=====================================================================================================
2) Add this to Common\Inc\Common\Render\RenderGraphPassList.h
=====================================================================================================

PASS(Texture2DArrayRW_PS)

=====================================================================================================
3) Add this to Browser\Lib\Renderer.cpp wherever you want the render pass to happen
=====================================================================================================

pass(&ITexture2DArrayRW_PSPass::addTexture2DArrayRW_PSPass, m_rgTexture2DArrayRW_PSPass);

If you are doing a post processing effect, a good place to put this could be right after the "final pass"
which is this line:  pass(&IFinalPass::addFinalPass, m_rgFinalPass);

=====================================================================================================
4) Put Texture2DArrayRW_PSPass.cpp and Texture2DArrayRW_PSPass.h and any custom c++ headers or
source files into Modules\Render.Passes\Lib\.
=====================================================================================================

=====================================================================================================
5) Put Texture2DArrayRW_PS_imgui.h into Browser\Lib.
=====================================================================================================

=====================================================================================================
6) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

//
struct Texture2DArrayRW_PSPassData
{
	// Set and get these
	RenderGraphResource textureNodeTexture;
	RenderGraphResource textureImportedTexture;
	glm::uvec3 textureImportedTexture_size = {0, 0, 0};
	RenderGraphResource textureImportedColor;
	glm::uvec3 textureImportedColor_size = {0, 0, 0};
	RenderGraphResource textureColor;
	glm::uvec3 textureColor_size = {0, 0, 0};

	// Internal use only
	RenderGraphResource texture_loadedTexture_0;
};
DeclareRenderGraphScopeContainer(Texture2DArrayRW_PSPassData);

=====================================================================================================
7) Hook up the inputs and outputs
=====================================================================================================

// Do this to get access to the pass data whenever you want to read or write it
auto& d = scope.get<Texture2DArrayRW_PSPassData>();

// Set the input resources
d.textureImportedTexture = /*Texture Goes Here*/;
d.textureImportedTexture_size = /*Texture Size Goes Here*/;
d.textureImportedColor = /*Texture Goes Here*/;
d.textureImportedColor_size = /*Texture Size Goes Here*/;
d.textureColor = /*Texture Goes Here*/;
d.textureColor_size = /*Texture Size Goes Here*/;

// Get and use the results
auto& d = scope.get<Texture2DArrayRW_PSPassData>();
// use d.textureImportedTexture for whatever you want
// use d.textureImportedColor for whatever you want
// use d.textureColor for whatever you want

If you are doing a post processing effect, a good way to hook up the inputs and outputs is to go into
your Texture2DArrayRW_PSPass::addTexture2DArrayRW_PSPass() function within Texture2DArrayRW_PSPass.cpp and put lines like these
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
8) Copy the contents of the shaders folder to Shaders\Texture2DArrayRW_PS\
=====================================================================================================

=====================================================================================================
9) Add the below to Shaders\Shaders.toml.
=====================================================================================================

=====================================================================================================
10) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

// This goes above struct RenderSettings
struct Texture2DArrayRW_PSSettings
{
};

// This goes in the struct RenderSettings
Texture2DArrayRW_PSSettings  Texture2DArrayRW_PS;

=====================================================================================================
11) Add this to Browser\Lib\BrowserGui.cpp
=====================================================================================================

// add to the top of the file
#include "Texture2DArrayRW_PS_imgui.h"

// put this into BrowserGui::doSettingsTweakerGui() wherever you want the UI to go, within the
// render settings section.  You can open the render settings in halcyon by selecting "Render Settings"
// in the Window menu.
Texture2DArrayRW_PS::MakeUI(m_owner.m_renderSettings.Texture2DArrayRW_PS);

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


