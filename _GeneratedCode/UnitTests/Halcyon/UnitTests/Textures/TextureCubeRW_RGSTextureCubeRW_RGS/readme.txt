Here are the instructions for how to use this package in Halcyon

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
1) Add this to Common\Inc\Common\Render\RenderGraphPassInterfaces.h
=====================================================================================================

class ITextureCubeRW_RGSPass : public IRenderPass
{
public:
	HCY_DECLARE_INTERFACE(ITextureCubeRW_RGSPass, 0);

	virtual void addTextureCubeRW_RGSPass(RenderGraph& renderGraph, RenderGraphScope& scope, const class ShaderPipelines& pipelines) = 0;
};

=====================================================================================================
2) Add this to Common\Inc\Common\Render\RenderGraphPassList.h
=====================================================================================================

PASS(TextureCubeRW_RGS)

=====================================================================================================
3) Add this to Browser\Lib\Renderer.cpp wherever you want the render pass to happen
=====================================================================================================

pass(&ITextureCubeRW_RGSPass::addTextureCubeRW_RGSPass, m_rgTextureCubeRW_RGSPass);

If you are doing a post processing effect, a good place to put this could be right after the "final pass"
which is this line:  pass(&IFinalPass::addFinalPass, m_rgFinalPass);

=====================================================================================================
4) Put TextureCubeRW_RGSPass.cpp and TextureCubeRW_RGSPass.h and any custom c++ headers or
source files into Modules\Render.Passes\Lib\.
=====================================================================================================

=====================================================================================================
5) Put TextureCubeRW_RGS_imgui.h into Browser\Lib.
=====================================================================================================

=====================================================================================================
6) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

//
struct TextureCubeRW_RGSPassData
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
DeclareRenderGraphScopeContainer(TextureCubeRW_RGSPassData);

=====================================================================================================
7) Hook up the inputs and outputs
=====================================================================================================

// Do this to get access to the pass data whenever you want to read or write it
auto& d = scope.get<TextureCubeRW_RGSPassData>();

// Set the input resources
d.textureImportedTexture = /*Texture Goes Here*/;
d.textureImportedTexture_size = /*Texture Size Goes Here*/;
d.textureImportedColor = /*Texture Goes Here*/;
d.textureImportedColor_size = /*Texture Size Goes Here*/;

// Get and use the results
auto& d = scope.get<TextureCubeRW_RGSPassData>();
// use d.textureImportedTexture for whatever you want
// use d.textureImportedColor for whatever you want

If you are doing a post processing effect, a good way to hook up the inputs and outputs is to go into
your TextureCubeRW_RGSPass::addTextureCubeRW_RGSPass() function within TextureCubeRW_RGSPass.cpp and put lines like these
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
8) Copy the contents of the shaders folder to Shaders\TextureCubeRW_RGS\
=====================================================================================================

=====================================================================================================
9) Add the below to Shaders\Shaders.toml.
=====================================================================================================

["Shaders/TextureCubeRW_RGS/RWCHS"]
type = ""
entryPoint = "chsmain"
source = "Shaders/TextureCubeRW_RGS/TextureCubeRW_CHS_MISS.hlsl"
flags = ""

["Shaders/TextureCubeRW_RGS/RWMISS"]
type = ""
entryPoint = "missmain"
source = "Shaders/TextureCubeRW_RGS/TextureCubeRW_CHS_MISS.hlsl"
flags = ""

["Shaders/TextureCubeRW_RGS/RW"]
type = ""
entryPoint = "rgsmain"
source = "Shaders/TextureCubeRW_RGS/TextureCubeRW_RGS.hlsl"
flags = ""

=====================================================================================================
10) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

// This goes above struct RenderSettings
struct TextureCubeRW_RGSSettings
{
};

// This goes in the struct RenderSettings
TextureCubeRW_RGSSettings  TextureCubeRW_RGS;

=====================================================================================================
11) Add this to Browser\Lib\BrowserGui.cpp
=====================================================================================================

// add to the top of the file
#include "TextureCubeRW_RGS_imgui.h"

// put this into BrowserGui::doSettingsTweakerGui() wherever you want the UI to go, within the
// render settings section.  You can open the render settings in halcyon by selecting "Render Settings"
// in the Window menu.
TextureCubeRW_RGS::MakeUI(m_owner.m_renderSettings.TextureCubeRW_RGS);

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


