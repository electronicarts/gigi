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

class MultipleUVMeshRenderPassModule : public RefCountImpl<IRenderPassModule>
{
public:
	MultipleUVMeshRenderPassModule() = default;
	virtual ~MultipleUVMeshRenderPassModule();

	void addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard);

	//IRenderPassModule interface
	void onInit(const InitParams& params) override;

public:
private:
	MemoryArena* m_arena = nullptr;

	static ShaderProgramPipelineCache s_pipelineCache;

	struct Struct_VertexBuffer
	{
		fb::math::Vector3Template<float> pos = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
		fb::math::Vector3Template<float> normal = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);
		fb::math::Vector2Template<float> UV0 = fb::math::Vector2Template<float>(0.f, 0.f);
		fb::math::Vector2Template<float> uv1 = fb::math::Vector2Template<float>(0.f, 0.f);
	};
};

}
