// (c) Electronic Arts.  All Rights Reserved.
#pragma once

#include <Engine.Render/Core2/Rc2Helpers.h>
#include <Engine.Render/FrameGraph/FrameGraphRenderPassModule.h>
#include <Engine.Render/Raytrace/RaytraceRenderPassModule.h>
#include <Engine.RtFramework/RtFramework.h>

#include <FBMath/Vector2TemplateType.h>
#include <FBMath/Vector3TemplateType.h>
#include <FBMath/Vector4TemplateType.h>
#include <FBMath/Matrix44TemplateType.h>

namespace fb
{

class StencilRenderPassModule : public RefCountImpl<IRenderPassModule>
{
public:
	StencilRenderPassModule() = default;
	virtual ~StencilRenderPassModule();

	void addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard);

	//IRenderPassModule interface
	void onInit(const InitParams& params) override;

public:

	// Host Variables
	fb::math::Vector4Template<float> m_varMouseState = fb::math::Vector4Template<float>(0.f, 0.f, 0.f, 0.f);
	fb::math::Vector4Template<float> m_varMouseStateLastFrame = fb::math::Vector4Template<float>(0.f, 0.f, 0.f, 0.f);
	fb::math::Vector3Template<float> m_variResolution = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
	float m_variTime = 0.f;
	float m_variTimeDelta = 0.f;
	float m_variFrameRate = 0.f;
	int m_variFrame = 0;
	fb::math::Vector4Template<float> m_variMouse = fb::math::Vector4Template<float>(0.f, 0.f, 0.f, 0.f);
	fb::math::Matrix44Template<float> m_varViewMtx = fb::math::Matrix44Template<float>(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);
	fb::math::Matrix44Template<float> m_varInvViewMtx = fb::math::Matrix44Template<float>(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);
	fb::math::Matrix44Template<float> m_varProjMtx = fb::math::Matrix44Template<float>(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);
	fb::math::Matrix44Template<float> m_varInvProjMtx = fb::math::Matrix44Template<float>(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);
	fb::math::Matrix44Template<float> m_varViewProjMtx = fb::math::Matrix44Template<float>(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);
	fb::math::Matrix44Template<float> m_varInvViewProjMtx = fb::math::Matrix44Template<float>(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);
	fb::math::Vector3Template<float> m_varCameraPos = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
	bool m_varCameraChanged = false;
private:
	MemoryArena* m_arena = nullptr;

	static ShaderProgramPipelineCache s_pipelineCache;

	struct Struct_VertexBuffer
	{
		fb::math::Vector3Template<float> pos = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
		fb::math::Vector3Template<float> normal = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
		fb::math::Vector2Template<float> uv = fb::math::Vector2Template<float>(0.f, 0.f);
	};

	struct Struct__Draw1VSCB
	{
		fb::math::Matrix44Template<float> ViewProjMtx = fb::math::Matrix44Template<float>(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);
	};

	SmartRef<ITexture> m_textureDepth_Stencil;
	RenderFormat m_textureDepth_Stencil_format = RenderFormat_Unknown;
	uint m_textureDepth_Stencil_width = 0;
	uint m_textureDepth_Stencil_height = 0;
	uint m_textureDepth_Stencil_depth = 0;

	Struct__Draw1VSCB m_cb__Draw1VSCB;
};

}
