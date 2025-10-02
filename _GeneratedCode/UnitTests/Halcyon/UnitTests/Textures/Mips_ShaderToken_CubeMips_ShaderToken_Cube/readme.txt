Here are the instructions for how to use this package in Halcyon

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
1) Add this to Common\Inc\Common\Render\RenderGraphPassInterfaces.h
=====================================================================================================

class IMips_ShaderToken_CubePass : public IRenderPass
{
public:
	HCY_DECLARE_INTERFACE(IMips_ShaderToken_CubePass, 0);

	virtual void addMips_ShaderToken_CubePass(RenderGraph& renderGraph, RenderGraphScope& scope, const class ShaderPipelines& pipelines) = 0;
};

=====================================================================================================
2) Add this to Common\Inc\Common\Render\RenderGraphPassList.h
=====================================================================================================

PASS(Mips_ShaderToken_Cube)

=====================================================================================================
3) Add this to Browser\Lib\Renderer.cpp wherever you want the render pass to happen
=====================================================================================================

pass(&IMips_ShaderToken_CubePass::addMips_ShaderToken_CubePass, m_rgMips_ShaderToken_CubePass);

If you are doing a post processing effect, a good place to put this could be right after the "final pass"
which is this line:  pass(&IFinalPass::addFinalPass, m_rgFinalPass);

=====================================================================================================
4) Put Mips_ShaderToken_CubePass.cpp and Mips_ShaderToken_CubePass.h and any custom c++ headers or
source files into Modules\Render.Passes\Lib\.
=====================================================================================================

=====================================================================================================
5) Put Mips_ShaderToken_Cube_imgui.h into Browser\Lib.
=====================================================================================================

=====================================================================================================
6) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

//
struct Mips_ShaderToken_CubePassData
{
	// Set and get these

	// Internal use only
	RenderGraphResource textureOutput;
	RenderGraphResource texture_loadedTexture_0;
};
DeclareRenderGraphScopeContainer(Mips_ShaderToken_CubePassData);

=====================================================================================================
7) Hook up the inputs and outputs
=====================================================================================================

// Do this to get access to the pass data whenever you want to read or write it
auto& d = scope.get<Mips_ShaderToken_CubePassData>();

// Set the input resources

// Get and use the results
auto& d = scope.get<Mips_ShaderToken_CubePassData>();

If you are doing a post processing effect, a good way to hook up the inputs and outputs is to go into
your Mips_ShaderToken_CubePass::addMips_ShaderToken_CubePass() function within Mips_ShaderToken_CubePass.cpp and put lines like these
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
8) Copy the contents of the shaders folder to Shaders\Mips_ShaderToken_Cube\
=====================================================================================================

=====================================================================================================
9) Add the below to Shaders\Shaders.toml.
=====================================================================================================

["Shaders/Mips_ShaderToken_Cube/MakeOutput"]
type = "cs"
entryPoint = "main"
source = "Shaders/Mips_ShaderToken_Cube/Mips_ShaderToken_Cube_CS.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

=====================================================================================================
10) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

// This goes above struct RenderSettings
struct Mips_ShaderToken_CubeSettings
{
};

// This goes in the struct RenderSettings
Mips_ShaderToken_CubeSettings  Mips_ShaderToken_Cube;

=====================================================================================================
11) Add this to Browser\Lib\BrowserGui.cpp
=====================================================================================================

// add to the top of the file
#include "Mips_ShaderToken_Cube_imgui.h"

// put this into BrowserGui::doSettingsTweakerGui() wherever you want the UI to go, within the
// render settings section.  You can open the render settings in halcyon by selecting "Render Settings"
// in the Window menu.
Mips_ShaderToken_Cube::MakeUI(m_owner.m_renderSettings.Mips_ShaderToken_Cube);

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


