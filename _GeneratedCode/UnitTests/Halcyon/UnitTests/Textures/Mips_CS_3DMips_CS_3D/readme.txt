Here are the instructions for how to use this package in Halcyon

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
1) Add this to Common\Inc\Common\Render\RenderGraphPassInterfaces.h
=====================================================================================================

class IMips_CS_3DPass : public IRenderPass
{
public:
	HCY_DECLARE_INTERFACE(IMips_CS_3DPass, 0);

	virtual void addMips_CS_3DPass(RenderGraph& renderGraph, RenderGraphScope& scope, const class ShaderPipelines& pipelines) = 0;
};

=====================================================================================================
2) Add this to Common\Inc\Common\Render\RenderGraphPassList.h
=====================================================================================================

PASS(Mips_CS_3D)

=====================================================================================================
3) Add this to Browser\Lib\Renderer.cpp wherever you want the render pass to happen
=====================================================================================================

pass(&IMips_CS_3DPass::addMips_CS_3DPass, m_rgMips_CS_3DPass);

If you are doing a post processing effect, a good place to put this could be right after the "final pass"
which is this line:  pass(&IFinalPass::addFinalPass, m_rgFinalPass);

=====================================================================================================
4) Put Mips_CS_3DPass.cpp and Mips_CS_3DPass.h and any custom c++ headers or
source files into Modules\Render.Passes\Lib\.
=====================================================================================================

=====================================================================================================
5) Put Mips_CS_3D_imgui.h into Browser\Lib.
=====================================================================================================

=====================================================================================================
6) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

//
struct Mips_CS_3DPassData
{
	// Set and get these
	RenderGraphResource textureOutput;

	// Internal use only
	RenderGraphResource texture_loadedTexture_0;
};
DeclareRenderGraphScopeContainer(Mips_CS_3DPassData);

=====================================================================================================
7) Hook up the inputs and outputs
=====================================================================================================

// Do this to get access to the pass data whenever you want to read or write it
auto& d = scope.get<Mips_CS_3DPassData>();

// Set the input resources

// Get and use the results
auto& d = scope.get<Mips_CS_3DPassData>();

If you are doing a post processing effect, a good way to hook up the inputs and outputs is to go into
your Mips_CS_3DPass::addMips_CS_3DPass() function within Mips_CS_3DPass.cpp and put lines like these
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
8) Copy the contents of the shaders folder to Shaders\Mips_CS_3D\
=====================================================================================================

=====================================================================================================
9) Add the below to Shaders\Shaders.toml.
=====================================================================================================

["Shaders/Mips_CS_3D/Mip0"]
type = "cs"
entryPoint = "main"
source = "Shaders/Mips_CS_3D/Mips_CS_3D_Mip0CS.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

["Shaders/Mips_CS_3D/Mip1"]
type = "cs"
entryPoint = "main"
source = "Shaders/Mips_CS_3D/Mips_CS_3D_MipNCS.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(2,2,2) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

["Shaders/Mips_CS_3D/Mip2"]
type = "cs"
entryPoint = "main"
source = "Shaders/Mips_CS_3D/Mips_CS_3D_MipNCS.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(4,4,4) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

["Shaders/Mips_CS_3D/Mip3"]
type = "cs"
entryPoint = "main"
source = "Shaders/Mips_CS_3D/Mips_CS_3D_MipNCS.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(8,8,8) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

=====================================================================================================
10) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

// This goes above struct RenderSettings
struct Mips_CS_3DSettings
{
	glm::uvec3 RenderSize = {256, 256, 8};
};

// This goes in the struct RenderSettings
Mips_CS_3DSettings  Mips_CS_3D;

=====================================================================================================
11) Add this to Browser\Lib\BrowserGui.cpp
=====================================================================================================

// add to the top of the file
#include "Mips_CS_3D_imgui.h"

// put this into BrowserGui::doSettingsTweakerGui() wherever you want the UI to go, within the
// render settings section.  You can open the render settings in halcyon by selecting "Render Settings"
// in the Window menu.
Mips_CS_3D::MakeUI(m_owner.m_renderSettings.Mips_CS_3D);

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


