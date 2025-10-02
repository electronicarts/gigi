Here are the instructions for how to use this package in Halcyon

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
1) Add this to Common\Inc\Common\Render\RenderGraphPassInterfaces.h
=====================================================================================================

class IVariableAliasesPass : public IRenderPass
{
public:
	HCY_DECLARE_INTERFACE(IVariableAliasesPass, 0);

	virtual void addVariableAliasesPass(RenderGraph& renderGraph, RenderGraphScope& scope, const class ShaderPipelines& pipelines) = 0;
};

=====================================================================================================
2) Add this to Common\Inc\Common\Render\RenderGraphPassList.h
=====================================================================================================

PASS(VariableAliases)

=====================================================================================================
3) Add this to Browser\Lib\Renderer.cpp wherever you want the render pass to happen
=====================================================================================================

pass(&IVariableAliasesPass::addVariableAliasesPass, m_rgVariableAliasesPass);

If you are doing a post processing effect, a good place to put this could be right after the "final pass"
which is this line:  pass(&IFinalPass::addFinalPass, m_rgFinalPass);

=====================================================================================================
4) Put VariableAliasesPass.cpp and VariableAliasesPass.h and any custom c++ headers or
source files into Modules\Render.Passes\Lib\.
=====================================================================================================

=====================================================================================================
5) Put VariableAliases_imgui.h into Browser\Lib.
=====================================================================================================

=====================================================================================================
6) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

//
struct VariableAliasesPassData
{
	// Set and get these
	RenderGraphResource textureColor;

	// Internal use only
};
DeclareRenderGraphScopeContainer(VariableAliasesPassData);

=====================================================================================================
7) Hook up the inputs and outputs
=====================================================================================================

// Do this to get access to the pass data whenever you want to read or write it
auto& d = scope.get<VariableAliasesPassData>();

// Set the input resources

// Get and use the results
auto& d = scope.get<VariableAliasesPassData>();

If you are doing a post processing effect, a good way to hook up the inputs and outputs is to go into
your VariableAliasesPass::addVariableAliasesPass() function within VariableAliasesPass.cpp and put lines like these
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
8) Copy the contents of the shaders folder to Shaders\VariableAliases\
=====================================================================================================

=====================================================================================================
9) Add the below to Shaders\Shaders.toml.
=====================================================================================================

["Shaders/VariableAliases/Set_Red"]
type = "cs"
entryPoint = "main"
source = "Shaders/VariableAliases/VariableAliases_SetChannel.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

["Shaders/VariableAliases/Set_Green"]
type = "cs"
entryPoint = "main"
source = "Shaders/VariableAliases/VariableAliases_SetChannel.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

["Shaders/VariableAliases/Set_Blue"]
type = "cs"
entryPoint = "main"
source = "Shaders/VariableAliases/VariableAliases_SetChannel.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0) __GIGI_AlIAS_VARIABLE_CONST_Value=0.75"

=====================================================================================================
10) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

// This goes above struct RenderSettings
struct VariableAliasesSettings
{
	glm::uvec2 RenderSize = {32, 32};
	int Node1Channel = 0;
	float Node1Value = 0.25f;
	int Node2Channel = 1;
	float Node2Value = 0.5f;
	int Node3Channel = 2;
	const float Node3Value = 0.75f;
	float UnusedFloat = 0.f;  // This is for the unused alias in the shader
};

// This goes in the struct RenderSettings
VariableAliasesSettings  VariableAliases;

=====================================================================================================
11) Add this to Browser\Lib\BrowserGui.cpp
=====================================================================================================

// add to the top of the file
#include "VariableAliases_imgui.h"

// put this into BrowserGui::doSettingsTweakerGui() wherever you want the UI to go, within the
// render settings section.  You can open the render settings in halcyon by selecting "Render Settings"
// in the Window menu.
VariableAliases::MakeUI(m_owner.m_renderSettings.VariableAliases);

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


