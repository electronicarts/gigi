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

class simpleRTRenderPassModule : public RefCountImpl<IRaytraceRenderPassModule>
{
public:
	simpleRTRenderPassModule() = default;
	virtual ~simpleRTRenderPassModule();

	void addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard);

	//IRenderPassModule interface
	void onInit(const InitParams& params) override;

	// IRaytraceRenderPassModule
	void onRegisterActiveWorlds(FrameGraphBlackboard& blackboard) override;

public:

	// Host Variables
	fb::math::Matrix44Template<float> m_varclipToWorld = fb::math::Matrix44Template<float>(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
	fb::math::Vector3Template<float> m_varcameraPos = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
	float m_vardepthNearPlane = 0.f;  // The depth value for the near plane.
private:
	MemoryArena* m_arena = nullptr;

	static ShaderProgramPipelineCache s_pipelineCache;

	struct Struct_VertexBuffer
	{
		fb::math::Vector3Template<float> Color = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
		fb::math::Vector3Template<float> Position = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
	};

	struct Struct_VertexBufferSimple
	{
		fb::math::Vector3Template<float> Position = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
	};

	struct Struct_VertexBufferFull
	{
		fb::math::Vector3Template<float> Position = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
		fb::math::Vector3Template<float> Color = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
		fb::math::Vector3Template<float> Normal = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
		fb::math::Vector4Template<float> Tangent = fb::math::Vector4Template<float>(0.f, 0.f, 0.f, 0.f);
		fb::math::Vector2Template<float> UV = fb::math::Vector2Template<float>(0.f, 0.f);
		int MaterialID = 0;
	};

	struct Struct__SimpleRTRayGenCB
	{
		fb::math::Vector3Template<float> cameraPos = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
		float _padding0 = 0.f;  // Padding
		fb::math::Matrix44Template<float> clipToWorld = fb::math::Matrix44Template<float>(0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
		float depthNearPlane = 0.f;  // The depth value for the near plane.
		fb::math::Vector3Template<float> hitColor = fb::math::Vector3Template<float>(0.f, 1.f, 0.f);
		fb::math::Vector3Template<float> missColor = fb::math::Vector3Template<float>(1.f, 0.f, 0.f);
		float _padding1 = 0.f;  // Padding
	};

	Struct__SimpleRTRayGenCB m_cb__SimpleRTRayGenCB;

	// Ray Shader: DoRT
	RtRef<RtTable> m_rayGenTable_DoRT;
	RtRef<RtBuffer> m_rayGenCB_DoRT;
	RtRef<RtTrace> m_rtTrace_DoRT;
};

}
