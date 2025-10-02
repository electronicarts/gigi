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

class ReadbackSequenceRenderPassModule : public RefCountImpl<IRenderPassModule>
{
public:
	ReadbackSequenceRenderPassModule() = default;
	virtual ~ReadbackSequenceRenderPassModule();

	void addPass(FrameGraph& frameGraph, FrameGraphBlackboard& blackboard);

	//IRenderPassModule interface
	void onInit(const InitParams& params) override;

public:

	// Host Variables
	int m_varframeIndex = 0;
private:
	MemoryArena* m_arena = nullptr;

	static ShaderProgramPipelineCache s_pipelineCache;

	struct Struct__ReadbackSequenceCS_0CB
	{
		int frameIndex = 0;
		fb::math::Vector3Template<float> _padding0 = fb::math::Vector3Template<float>(0.f, 0.f, 0.f);  // Padding
	};

	Struct__ReadbackSequenceCS_0CB m_cb__ReadbackSequenceCS_0CB;
};

}
