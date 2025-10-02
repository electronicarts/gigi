Here are the instructions for how to use this package in Halcyon

Note: The graphviz folder (if present) is there to show the render graph before and after optimization and flattening

=====================================================================================================
1) Add this to Common\Inc\Common\Render\RenderGraphPassInterfaces.h
=====================================================================================================

class IYesVertexStruct_NoIndex_NoInstancePass : public IRenderPass
{
public:
	HCY_DECLARE_INTERFACE(IYesVertexStruct_NoIndex_NoInstancePass, 0);

	virtual void addYesVertexStruct_NoIndex_NoInstancePass(RenderGraph& renderGraph, RenderGraphScope& scope, const class ShaderPipelines& pipelines) = 0;
};

=====================================================================================================
2) Add this to Common\Inc\Common\Render\RenderGraphPassList.h
=====================================================================================================

PASS(YesVertexStruct_NoIndex_NoInstance)

=====================================================================================================
3) Add this to Browser\Lib\Renderer.cpp wherever you want the render pass to happen
=====================================================================================================

pass(&IYesVertexStruct_NoIndex_NoInstancePass::addYesVertexStruct_NoIndex_NoInstancePass, m_rgYesVertexStruct_NoIndex_NoInstancePass);

If you are doing a post processing effect, a good place to put this could be right after the "final pass"
which is this line:  pass(&IFinalPass::addFinalPass, m_rgFinalPass);

=====================================================================================================
4) Put YesVertexStruct_NoIndex_NoInstancePass.cpp and YesVertexStruct_NoIndex_NoInstancePass.h and any custom c++ headers or
source files into Modules\Render.Passes\Lib\.
=====================================================================================================

=====================================================================================================
5) Put YesVertexStruct_NoIndex_NoInstance_imgui.h into Browser\Lib.
=====================================================================================================

=====================================================================================================
6) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

//
struct YesVertexStruct_NoIndex_NoInstancePassData
{
	// Set and get these
	RenderGraphResource textureColor_Buffer;
	RenderGraphResource textureDepth_Buffer;
	RenderGraphResource bufferVertex_Buffer;

	// Internal use only
};
DeclareRenderGraphScopeContainer(YesVertexStruct_NoIndex_NoInstancePassData);

=====================================================================================================
7) Hook up the inputs and outputs
=====================================================================================================

// Do this to get access to the pass data whenever you want to read or write it
auto& d = scope.get<YesVertexStruct_NoIndex_NoInstancePassData>();

// Set the input resources
d.bufferVertex_Buffer = /*Buffer Goes Here*/;

// Get and use the results
auto& d = scope.get<YesVertexStruct_NoIndex_NoInstancePassData>();
// use d.bufferVertex_Buffer for whatever you want

If you are doing a post processing effect, a good way to hook up the inputs and outputs is to go into
your YesVertexStruct_NoIndex_NoInstancePass::addYesVertexStruct_NoIndex_NoInstancePass() function within YesVertexStruct_NoIndex_NoInstancePass.cpp and put lines like these
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
8) Copy the contents of the shaders folder to Shaders\YesVertexStruct_NoIndex_NoInstance\
=====================================================================================================

=====================================================================================================
9) Add the below to Shaders\Shaders.toml.
=====================================================================================================

=====================================================================================================
10) Add this to Common\Inc\Common\Render\RenderPassPayload.h
=====================================================================================================

// This goes above struct RenderSettings
struct YesVertexStruct_NoIndex_NoInstanceSettings
{

	enum class ViewMode: int
	{
		Normal,
		UV,
		Solid,
	};
	glm::vec4 MouseState = {0.f, 0.f, 0.f, 0.f};
	glm::vec4 MouseStateLastFrame = {0.f, 0.f, 0.f, 0.f};
	glm::vec3 iResolution = {0.f, 0.f, 0.f};
	float iTime = 0.f;
	float iTimeDelta = 0.f;
	float iFrameRate = 0.f;
	int iFrame = 0;
	glm::vec4 iMouse = {0.f, 0.f, 0.f, 0.f};
	glm::mat4 ViewMtx = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
	glm::mat4 InvViewMtx = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
	glm::mat4 ProjMtx = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
	glm::mat4 InvProjMtx = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
	glm::mat4 ViewProjMtx = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
	glm::mat4 InvViewProjMtx = {1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f};
	glm::vec3 CameraPos = {0.f, 0.f, 0.f};
	ViewMode viewMode = ViewMode::Normal;
};

// This goes in the struct RenderSettings
YesVertexStruct_NoIndex_NoInstanceSettings  YesVertexStruct_NoIndex_NoInstance;

=====================================================================================================
11) Add this to Browser\Lib\BrowserGui.cpp
=====================================================================================================

// add to the top of the file
#include "YesVertexStruct_NoIndex_NoInstance_imgui.h"

// put this into BrowserGui::doSettingsTweakerGui() wherever you want the UI to go, within the
// render settings section.  You can open the render settings in halcyon by selecting "Render Settings"
// in the Window menu.
YesVertexStruct_NoIndex_NoInstance::MakeUI(m_owner.m_renderSettings.YesVertexStruct_NoIndex_NoInstance);

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


