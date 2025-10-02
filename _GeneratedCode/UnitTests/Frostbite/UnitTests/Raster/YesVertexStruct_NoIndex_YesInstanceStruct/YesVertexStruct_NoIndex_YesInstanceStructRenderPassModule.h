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

class YesVertexStruct_NoIndex_YesInstanceStructRenderPassModule : public RefCountImpl<IRenderPassModule>
{
public:
	YesVertexStruct_NoIndex_YesInstanceStructRenderPassModule() = default;
	virtual ~YesVertexStruct_NoIndex_YesInstanceStructRenderPassModule();

	void addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard);

	//IRenderPassModule interface
	void onInit(const InitParams& params) override;

public:

	enum class ViewMode: int
	{
		Normal,
		UV,
		Solid,
	};

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
private:
	MemoryArena* m_arena = nullptr;

	static ShaderProgramPipelineCache s_pipelineCache;

	struct Struct_VertexFormat
	{
		fb::math::Vector3Template<float> Position = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
		fb::math::Vector3Template<float> Normal = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
		fb::math::Vector2Template<float> UV = fb::math::Vector2Template<float>(0.f, 0.f);
	};

	struct Struct_InstanceBufferFormat
	{
		fb::math::Vector3Template<float> Offset = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
		float Scale = 0.f;
	};

	struct Struct__VertexShaderCB
	{
		fb::math::Matrix44Template<float> ViewProjMtx = fb::math::Matrix44Template<float>(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);
	};

	struct Struct__PixelShaderCB
	{
		int viewMode = (int)ViewMode::Normal;
		fb::math::Vector3Template<float> _padding0 = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);  // Padding
	};

	Struct__VertexShaderCB m_cb__VertexShaderCB;

	Struct__PixelShaderCB m_cb__PixelShaderCB;
};

}
