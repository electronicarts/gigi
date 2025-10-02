Here are the instructions for how to use this package in Halcyon

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
1) Add this to Common\Inc\Common\Render\RenderGraphPassInterfaces.h
=====================================================================================================

class IbuffertestPass : public IRenderPass
{
public:
	HCY_DECLARE_INTERFACE(IbuffertestPass, 0);

	virtual void addbuffertestPass(RenderGraph& renderGraph, RenderGraphScope& scope, const class ShaderPipelines& pipelines) = 0;
};

=====================================================================================================
2) Add this to Common\Inc\Common\Render\RenderGraphPassList.h
=====================================================================================================

PASS(buffertest)

=====================================================================================================
3) Add this to Browser\Lib\Renderer.cpp wherever you want the render pass to happen
=====================================================================================================

pass(&IbuffertestPass::addbuffertestPass, m_rgbuffertestPass);

If you are doing a post processing effect, a good place to put this could be right after the "final pass"
which is this line:  pass(&IFinalPass::addFinalPass, m_rgFinalPass);

=====================================================================================================
4) Put buffertestPass.cpp and buffertestPass.h and any custom c++ headers or
source files into Modules\Render.Passes\Lib\.
=====================================================================================================

=====================================================================================================
5) Put buffertest_imgui.h into Browser\Lib.
=====================================================================================================

=====================================================================================================
6) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

//
struct buffertestPassData
{
	// Set and get these
	RenderGraphResource bufferInputTypedBuffer;
	RenderGraphResource bufferOutputTypedBuffer;
	RenderGraphResource bufferInputStructuredBuffer;
	RenderGraphResource bufferOutputStructuredBuffer;
	RenderGraphResource bufferInputTypedBufferRaw;
	RenderGraphResource bufferOutputTypedBufferRaw;
	RenderGraphResource bufferInputTypedStructBuffer;
	RenderGraphResource bufferOutputTypedStructBuffer;

	// Internal use only
};
DeclareRenderGraphScopeContainer(buffertestPassData);

=====================================================================================================
7) Hook up the inputs and outputs
=====================================================================================================

// Do this to get access to the pass data whenever you want to read or write it
auto& d = scope.get<buffertestPassData>();

// Set the input resources
d.bufferInputTypedBuffer = /*Buffer Goes Here*/;
d.bufferInputStructuredBuffer = /*Buffer Goes Here*/;
d.bufferInputTypedBufferRaw = /*Buffer Goes Here*/;
d.bufferInputTypedStructBuffer = /*Buffer Goes Here*/;

// Get and use the results
auto& d = scope.get<buffertestPassData>();
// use d.bufferInputTypedBuffer for whatever you want
// use d.bufferInputStructuredBuffer for whatever you want
// use d.bufferInputTypedBufferRaw for whatever you want
// use d.bufferInputTypedStructBuffer for whatever you want

If you are doing a post processing effect, a good way to hook up the inputs and outputs is to go into
your buffertestPass::addbuffertestPass() function within buffertestPass.cpp and put lines like these
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
8) Copy the contents of the shaders folder to Shaders\buffertest\
=====================================================================================================

=====================================================================================================
9) Add the below to Shaders\Shaders.toml.
=====================================================================================================

["Shaders/buffertest/BufferTest"]
type = "cs"
entryPoint = "Main"
source = "Shaders/buffertest/BufferTest.hlsl"
flags = ""
defines="__GigiDispatchMultiply=uint3(1,1,1) __GigiDispatchDivide=uint3(1,1,1) __GigiDispatchPreAdd=uint3(0,0,0) __GigiDispatchPostAdd=uint3(0,0,0)"

=====================================================================================================
10) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

// This goes above struct RenderSettings
struct buffertestSettings
{
	float gain = 0.5f;  // Overall Volume Adjustment
	float alpha1 = 1.f;  // Adjusts the contribution of sample n-1
	float alpha2 = 0.f;  // Adjusts the contribution of sample n-2
};

// This goes in the struct RenderSettings
buffertestSettings  buffertest;

=====================================================================================================
11) Add this to Browser\Lib\BrowserGui.cpp
=====================================================================================================

// add to the top of the file
#include "buffertest_imgui.h"

// put this into BrowserGui::doSettingsTweakerGui() wherever you want the UI to go, within the
// render settings section.  You can open the render settings in halcyon by selecting "Render Settings"
// in the Window menu.
buffertest::MakeUI(m_owner.m_renderSettings.buffertest);

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


